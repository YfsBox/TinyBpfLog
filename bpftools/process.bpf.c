//
// Created by 杨丰硕 on 2022/11/23.
//
// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright (c) 2020 Facebook */
#include "vmlinux/vmlinux.h"
#include "libbpf/src/bpf_helpers.h"
#include "libbpf/src/bpf_tracing.h"
#include "libbpf/src/bpf_core_read.h"
#include "process.h"

char LICENSE[] SEC("license") = "Dual BSD/GPL";

// 为什么非要定义这些map而不是使用自己定义的数据结构存储?因为我们自己定义的数据结构是处于用户态空间的
// 所以得专门借助.maps,因为这是处于内核空间中的数据结构.
// 定义了bpf map数据结构,其中有type,max_entries,key,value这几种属性的设置
// 下面分别定义了一个kvtable一个buffer类型的数据结构
// 其中,我对于其max_entries的大小选择的考虑存在疑惑
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 8192);
    __type(key, pid_t);
    __type(value, u64);
} exec_start SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 256 * 1024);
} rb SEC(".maps");  // 这个buf往往是作为内核太到用户态的桥梁的.

const volatile unsigned long long min_duration_ns = 0;

SEC("tp/sched/sched_process_exec")
int handle_exec(struct trace_event_raw_sched_process_exec *ctx) // 关于这个参数来源于linux内核,至于我们在定义handle时
// 选择使用哪种类型的参数,这一点应该如何了解??
{
    struct task_struct *task;
    unsigned fname_off;
    struct process_event *e;
    pid_t pid;
    u64 ts;

    /* remember time exec() was executed for this PID */
    pid = bpf_get_current_pid_tgid() >> 32;
    ts = bpf_ktime_get_ns();
    bpf_map_update_elem(&exec_start, &pid, &ts, BPF_ANY); // 将该数据更新到之前定义的map中.
    /* don't emit exec events when minimum duration is specified */
    if (min_duration_ns)
        return 0;
    /* reserve sample from BPF ringbuf */
    // 这里考虑用一个buffer来存储e,首先留出一个空位,内部的值还没有被定义.
    e = bpf_ringbuf_reserve(&rb, sizeof(*e), 0);
    if (!e)
        return 0;
    // 获取当前的进程,也就是task
    /* fill out the sample with data */
    task = (struct task_struct *)bpf_get_current_task();

    e->exit_event = false;
    e->pid = pid;
    e->ppid = BPF_CORE_READ(task, real_parent, tgid);
    bpf_get_current_comm(&e->comm, sizeof(e->comm));

    fname_off = ctx->__data_loc_filename & 0xFFFF;
    bpf_probe_read_str(&e->filename, sizeof(e->filename), (void *)ctx + fname_off);

    /* successfully submit it to user-space for post-processing */
    bpf_ringbuf_submit(e, 0);
    return 0;
}

SEC("tp/sched/sched_process_exit")
int handle_exit(struct trace_event_raw_sched_process_template* ctx)
{
    struct task_struct *task;
    struct process_event *e;
    pid_t pid, tid;
    u64 id, ts, *start_ts, duration_ns = 0;
    /* get PID and TID of exiting thread/process */
    id = bpf_get_current_pid_tgid();  // 这也就是对应了触发process_exit的进程
    pid = id >> 32;
    tid = (u32)id;

    /* ignore thread exits */
    if (pid != tid)
        return 0;
    /* if we recorded start of the process, calculate lifetime duration */
    start_ts = bpf_map_lookup_elem(&exec_start, &pid);
    if (start_ts)
        duration_ns = bpf_ktime_get_ns() - *start_ts;
    else if (min_duration_ns)
        return 0;
    bpf_map_delete_elem(&exec_start, &pid);

    /* if process didn't live long enough, return early */
    if (min_duration_ns && duration_ns < min_duration_ns)
        return 0;
    /* reserve sample from BPF ringbuf */
    e = bpf_ringbuf_reserve(&rb, sizeof(*e), 0);
    if (!e)
        return 0;

    /* fill out the sample with data */
    task = (struct task_struct *) bpf_get_current_task(); // 仍然是上面求的一个进程
    // 根据task中的字段设置event中的属性,这个communation表示的是什么?
    e->exit_event = true;
    e->duration_ns = duration_ns;
    e->pid = pid;
    e->ppid = BPF_CORE_READ(task, real_parent, tgid);
    e->exit_code = (BPF_CORE_READ(task, exit_code) >> 8) & 0xff;
    bpf_get_current_comm(&e->comm, sizeof(e->comm));
    /* send data to user-space for post-processing */
    bpf_ringbuf_submit(e, 0); // 提交到用户态
    return 0;
}

// 关于SEC中的字符串的内容,我认为除了一开始表示的“类型”,之外剩余的部分是与内核相关的.
// 通过这个demo,学习如何定义内核数据结构.map等.然后在定义的tracepoint中,如何在定义的handle中获取其中的一些内核信息
// 对于这些内核信息的获取,很多时候接住了一些bpf的API,比如说bpf_get_current_task等.除此之外,还需要将收集的内核
// 中的一些信息先写到内核中定义的数据结构中(.map),写完之后,最后一般再传递到用户态空间,一般借助ringbuf完成,这一步.
// 在这个过程中,应该根据自己的需要了解一些必要的内核数据结构以及bpf API.尤其是对于handle中的参数的选择问题,我们该如何知晓?


//