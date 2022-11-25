//
// Created by 杨丰硕 on 2022/11/25.
//

#ifndef TINYBPFLOG_MONITORS_H
#define TINYBPFLOG_MONITORS_H

#include <argp.h>
#include <pthread.h>
#include "common.h"
#include "process.h"
#include "ipc.h"

int process_handle_event(void *ctx, void *data, size_t data_sz);
int start_process_monitor(ring_buffer_sample_fn handle_event);
int idc_handle_event(void *ctx, void *data, size_t data_sz);
int start_ipc_monitor(ring_buffer_sample_fn handle_event);


#endif //TINYBPFLOG_MONITORS_H
