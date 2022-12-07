//
// Created by 杨丰硕 on 2022/11/29.
//
#include <memory>
#include <unordered_map>
#include <arpa/inet.h>
#include "common.h"
#include "monitors.h"
#include "tcpstate.h"
#include "tcpstate.skel.h"
#include "../runtime/NanoLogCpp17.h"

#define PERF_POLL_TIMEOUT_MS 100

static std::unordered_map<int, std::string> tcpstatesMap = {
        {1, "ESTABLISHED"},
        {2, "SYN_SENT" },
        {3, "SYN_RECV"},
        {4, "FIN_WAIT1"},
        {5, "FIN_WAIT2"},
        {6, "TIME_WAIT"},
        {7, "CLOSE"},
        {8, "CLOSE_WAIT"},
        {9, "LAST_ACK"},
        {10, "LISTEN"},
        {11, "CLOSING"},
        {12, "NEW_SYN_RECV"},
        {13, "UNKNOWN"},
};

TcpStateConfig::TcpStateConfig(uint32_t monitorId):
        Config(monitorId){
}

TcpStateConfig::~TcpStateConfig() = default;

bool TcpStateConfig::SetConfig() {
    return true;
}

shptrTcpStateConfig tcp_config;

static void tcpstate_handle_event(void *ctx, int cpu, void *data, __u32 data_sz) {
    char ts[32], saddr[26], daddr[26];
    tcpstate_event *event = reinterpret_cast<tcpstate_event *>(data);
    struct tm *tm;
    int family;
    time_t t;

    if (tcp_config->GetEmitTimestamp()) {
        time(&t);
        tm = localtime(&t);
        strftime(ts, sizeof(ts), "%H:%M:%S", tm);
        printf("%8s ", ts);
    }

    inet_ntop(event->family, &event->saddr, saddr, sizeof(saddr));
    inet_ntop(event->family, &event->daddr, daddr, sizeof(daddr));
    if (tcp_config->GetWideOutput()) {
        family = event->family == AF_INET ? 4 : 6;
        NANO_LOG(NOTICE, "%-16llx %-7d %-16s %-2d %-26s %-5d %-26s %-5d %-11s -> %-11s %.3f",
               event->skaddr, event->pid, event->task, family, saddr, event->sport, daddr, event->dport,
               tcpstatesMap[event->oldstate].c_str(), tcpstatesMap[event->newstate].c_str(), static_cast<double>(event->delta_us) / 1000);
    } else {
        NANO_LOG(NOTICE, "%-16llx %-7d %-10.10s %-15s %-5d %-15s %-5d %-11s -> %-11s %.3f",
               event->skaddr, event->pid, event->task, saddr, event->sport, daddr, event->dport,
               tcpstatesMap[event->oldstate].c_str(), tcpstatesMap[event->newstate].c_str(), static_cast<double>(event->delta_us) / 1000);
    }
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

    if (err = tcpstate_bpf__load(obj); err) {
        fprintf(stderr, "Failed to load and verify BPF skeleton\n");
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




