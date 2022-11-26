//
// Created by 杨丰硕 on 2022/11/26.
//
#ifndef TINYBPFLOG_MOUNT_H
#define TINYBPFLOG_MOUNT_H

#define TASK_COMM_LEN 16
#define FS_NAME_LEN 8
#define DATA_LEN 512
#define PATH_MAX 4096

enum mount_op {
    MOUNT = 0,
    UMOUNT,
};

struct mount_event {
    unsigned long long delta;
    unsigned long long flags;
    unsigned int pid;
    unsigned int tid;
    unsigned int mnt_ns;
    int ret;
    char comm[TASK_COMM_LEN];
    char fs[FS_NAME_LEN];
    char src[PATH_MAX];
    char dest[PATH_MAX];
    char data[DATA_LEN];
    enum mount_op op;
};

#endif //TINYBPFLOG_MOUNT_H
