//
// Created by 杨丰硕 on 2022/11/23.
//
#ifndef TINYBPFLOG_PROCESS_H
#define TINYBPFLOG_PROCESS_H
/* SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause) */
/* Copyright (c) 2020 Facebook */
#define TASK_COMM_LEN 16
#define MAX_FILENAME_LEN 127

struct process_event {
    int pid; // 这个ok
    int ppid;
    unsigned exit_code;
    unsigned long long duration_ns; // 这个也可ok
    char comm[TASK_COMM_LEN];
    char filename[MAX_FILENAME_LEN];  // 这个也ok
    bool exit_event;
};  // 对于pid和ppid来说

#endif //TINYBPFLOG_PROCESS_H
