//
// Created by 杨丰硕 on 2022/11/29.
//
#include <memory>
#include "common.h"
#include "monitors.h"
#include "tcpstate.skel.h"
#include "../runtime/NanoLogCpp17.h"

#define PERF_POLL_TIMEOUT_MS 100

TcpStateConfig::TcpStateConfig(uint32_t monitorId):
        Config(monitorId){
}

TcpStateConfig::~TcpStateConfig() = default;

bool TcpStateConfig::SetConfig() {
    return true;
}

shptrTcpStateConfig tcp_config;

static void tcpstate_handle_event(void *ctx, int cpu, void *data, __u32 data_sz) {

    return;
}

int start_tcpstate_monitor(ring_buffer_sample_fn handle_event, const shptrConfig &config) {
    perf_buffer *pb = nullptr;
    perf_buffer_opts pb_opts = {};

    struct tcpstate_bpf *obj = nullptr;
    int err;
    tcp_config = std::dynamic_pointer_cast<TcpStateConfig>(config);

    libbpf_set_strict_mode(LIBBPF_STRICT_ALL);
    libbpf_set_print(libbpf_print_fn);
    bump_memlock_rlimit();

    if (obj = tcpstate_bpf__open(); !obj) {
        // warn("failed to open BPF object\n");
        printf("failed to open BPF object\n");
        goto cleanup;
    }
    if (err = tcpstate_bpf__attach(obj); err) {
        // warn("failed to attach BPF programs: %d\n", err);
        printf("failed to attach BPF programs: %d\n", err);
        goto cleanup;
    }

    pb_opts.sample_cb = tcpstate_handle_event;
    pb_opts.lost_cb = handle_lost_events;
    if (pb = perf_buffer__new(bpf_map__fd(obj->maps.events), 8, &pb_opts); !pb) {
        err = -errno;
        warn("failed to open perf buffer: %d\n", err);
        goto cleanup;
    }
    while (!tcp_config->IsExit()) {
        err = perf_buffer__poll(pb, PERF_POLL_TIMEOUT_MS);
        if (err < 0 && err != -EINTR) {
            fprintf(stderr, "error polling perf buffer: %s\n", strerror(-err));
            goto cleanup;
        }
        /* reset err to return 0 if exiting */
        err = 0;
    }
    cleanup:
        perf_buffer__free(pb);
        tcpstate_bpf__destroy(obj);
    return 0;
}




