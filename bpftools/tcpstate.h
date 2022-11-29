//
// Created by 杨丰硕 on 2022/11/29.
//
#ifndef TINYBPFLOG_TCPSTATE_H
#define TINYBPFLOG_TCPSTATE_H

#define TASK_COMM_LEN 16

struct tcpstate_event {
    unsigned __int128 saddr;
    unsigned __int128 daddr;
    __u64 skaddr;
    __u64 ts_us;
    __u64 delta_us;
    __u32 pid;
    int oldstate;
    int newstate;
    __u16 family;
    __u16 sport;
    __u16 dport;
    char task[TASK_COMM_LEN];
};

#endif //TINYBPFLOG_TCPSTATE_H
