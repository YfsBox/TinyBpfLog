//
// Created by 杨丰硕 on 2022/11/23.
//

#ifndef TINYBPFLOG_PROCESS_MONITOR_H
#define TINYBPFLOG_PROCESS_MONITOR_H

#include <argp.h>
#include <pthread.h>
#include "common.h"
#include "process.h"


int process_handle_event(void *ctx, void *data, size_t data_sz);
int start_process_monitor(ring_buffer_sample_fn handle_event);



#endif //TINYBPFLOG_PROCESS_MONITOR_H
