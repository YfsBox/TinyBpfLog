//
// Created by 杨丰硕 on 2022/11/23.
//
// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright (c) 2020 Facebook */
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

char LICENSE[] SEC("license") = "Dual BSD/GPL";

int my_pid = 0;

// tp也就是tracepoint相当于断点的意思,syscall表示的是该断点的类型为系统调用
// 其中的系统调用类型就是,sys_enter_write,对应的系统调用为write
SEC("tp/syscalls/sys_enter_write")
int handle_tp(void *ctx)
{
    int pid = bpf_get_current_pid_tgid() >> 32;  // bpf标准库中提供的函数,返回触发该系统调用的进程的pid

    if (pid != my_pid) // 这个my_pid究竟指的是什么??
        return 0;

    bpf_printk("BPF triggered from PID %d.\n", pid); // 打断点,当执行到write对应的事件的时候,就会出发这些动作,其中bpf_printk

    return 0;
}

// bpf_printk究竟指的是什么??

