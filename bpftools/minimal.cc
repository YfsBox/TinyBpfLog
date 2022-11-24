//
// Created by 杨丰硕 on 2022/11/23.
//

// SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause)
/* Copyright (c) 2020 Facebook */
// 这个文件对应了minimal的用户态程序
#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>
#include <bpf/libbpf.h>
#include "minimal.skel.h"

//总体来说,minimal是一个用来在write系统调用上打断点的程序.这个程序是一个用户态程序.
static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
    return vfprintf(stderr, format, args); // 这个vfprintf应该是一个内核中的函数吧
}

static void bump_memlock_rlimit(void)
{
    struct rlimit rlim_new = {
            .rlim_cur	= RLIM_INFINITY,
            .rlim_max	= RLIM_INFINITY,
    };

    if (setrlimit(RLIMIT_MEMLOCK, &rlim_new)) {
        fprintf(stderr, "Failed to increase RLIMIT_MEMLOCK limit!\n");
        exit(1);
    }
}

int main(int argc, char **argv)
{
    struct minimal_bpf *skel;
    int err;

    /* Set up libbpf errors and debug info callback */
    libbpf_set_print(libbpf_print_fn);
    /* Bump RLIMIT_MEMLOCK to allow BPF sub-system to do anything */
    bump_memlock_rlimit();

    /* Open BPF application */
    skel = minimal_bpf__open();
    if (!skel) {
        fprintf(stderr, "Failed to open BPF skeleton\n");
        return 1;
    }

    /* ensure BPF program only handles write() syscalls from our process */
    skel->bss->my_pid = getpid();
    /* Load & verify BPF programs */
    err = minimal_bpf__load(skel); // 将bpf程序载入并且通过内核的verify,返回值如果为0,就说明已经通过ok
    if (err) {
        fprintf(stderr, "Failed to load and verify BPF skeleton\n");
        goto cleanup;
    }

    /* Attach tracepoint handler */
    err = minimal_bpf__attach(skel); // 将其中的handle加入
    if (err) {
        fprintf(stderr, "Failed to attach BPF skeleton\n");
        goto cleanup;
    }

    printf("Successfully started! Please run `sudo cat /sys/kernel/debug/tracing/trace_pipe` "
           "to see output of the BPF programs.\n");

    for (;;) {
        /* trigger our BPF program */
        fprintf(stderr, ".");
        sleep(1);
    }

    cleanup:
    minimal_bpf__destroy(skel); // 对skel进行清理
    return -err;
}

// 似乎很多用户态程序都有libbpf_print_fn和bump_memlock_rlimit这究竟指的是什么

// 对于生成的ebpf文件,似乎都会有一个结构体,关于这个结构体的信息.

// 其中trace_pipe的作用在于什么?

