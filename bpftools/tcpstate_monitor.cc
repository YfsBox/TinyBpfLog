//
// Created by 杨丰硕 on 2022/11/29.
//
#include <memory>
#include "common.h"
#include "monitors.h"
#include "tcpstate.skel.h"
#include "../runtime/NanoLogCpp17.h"

TcpStateConfig::TcpStateConfig(uint32_t monitorId):
        Config(monitorId){
}

TcpStateConfig::~TcpStateConfig() = default;

bool TcpStateConfig::SetConfig() {
    return true;
}

static void tcpstate_handle_event(void *ctx, int cpu, void *data, __u32 data_sz) {

    return;
}

int start_tcpstate_monitor(ring_buffer_sample_fn handle_event, const shptrConfig &cofig) {
    perf_buffer *pb = nullptr;
    perf_buffer_opts pb_opts = {};

    struct tcpstate_bpf *obj;
    int err;


    return 0;
}




