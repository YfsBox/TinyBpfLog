//
// Created by 杨丰硕 on 2022/11/23.
//

#ifndef TINYBPFLOG_COMMON_H
#define TINYBPFLOG_COMMON_H

#include <argp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>
#include "libbpf/src/libbpf.h"

static struct env {
    bool verbose;
    long min_duration_ms;
} env;

int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args);
void bump_memlock_rlimit(void);


#endif //TINYBPFLOG_COMMON_H
