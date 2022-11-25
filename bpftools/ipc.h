//
// Created by 杨丰硕 on 2022/11/25.
//

#ifndef TINYBPFLOG_IPC_H
#define TINYBPFLOG_IPC_H

struct ipc_event {
    unsigned int pid;
    unsigned int uid;
    unsigned int gid;
    unsigned int cuid;
    unsigned int cgid;
};

#endif //TINYBPFLOG_IPC_H
