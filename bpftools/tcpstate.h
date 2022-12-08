//
// Created by 杨丰硕 on 2022/11/29.
//
#ifndef TINYBPFLOG_TCPSTATE_H
#define TINYBPFLOG_TCPSTATE_H

#define TASK_COMM_LEN 16

struct tcpstate_event {
    unsigned __int128 saddr;
    unsigned __int128 daddr;
    uint64_t skaddr;
    uint64_t ts_us;
    uint64_t delta_us;
    uint32_t pid;
    int oldstate;
    int newstate;
    uint16_t family;
    uint16_t sport;
    uint16_t dport;
    char task[TASK_COMM_LEN];
};

#endif //TINYBPFLOG_TCPSTATE_H
