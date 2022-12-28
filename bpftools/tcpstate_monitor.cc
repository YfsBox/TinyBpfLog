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

TcpStateConfig::TcpStateConfig(const std::string &monitorId,
                               const std::string &monitorName,
                               bool emit_timestamp, bool wide_output,
                               bool saddr_eb, bool pid_eb,
                               bool sport_eb, bool dport_eb): Config(monitorId, monitorName),
                               emit_timestamp_(emit_timestamp),
                               wide_output_(wide_output),
                               saddr_enable_(saddr_eb),
                               pid_enable_(pid_eb),
                               sport_enable_(sport_eb),
                               dport_enable_(dport_eb)
                               {

}

TcpStateConfig::~TcpStateConfig() = default;
void TcpStateConfig::AddSaddr(unsigned __int128 saddr) {
    if (!saddr_enable_) {
        saddr_enable_.store(true);
    }
    std::lock_guard<std::mutex> guard(mutex_);
    saddrWhiteSet_.insert(saddr);
}

void TcpStateConfig::AddPid(uint32_t pid) {
    if (!pid_enable_) {
        pid_enable_.store(true);
    }
    std::lock_guard<std::mutex> guard(mutex_);
    pidWhiteSet_.insert(pid);
}

void TcpStateConfig::AddSport(uint16_t sport) {
    if (!sport_enable_) {
        sport_enable_.store(true);
    }
    std::lock_guard<std::mutex> guard(mutex_);
    sportWhiteSet_.insert(sport);
}

void TcpStateConfig::AddDport(uint16_t dport) {
    if (!dport_enable_) {
        dport_enable_.store(true);
    }
    std::lock_guard<std::mutex> guard(mutex_);
    dportWhiteSet_.insert(dport);
}

void TcpStateConfig::ShowConfig() { // 用来作为测试helper函数
    uint32_t sasize, psize, spsize, dpsize;
    bool saeb = saddr_enable_, peb = pid_enable_,
            speb = sport_enable_, dpeb = dport_enable_;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        sasize = saddrWhiteSet_.size();
        psize = pidWhiteSet_.size();
        spsize = sportWhiteSet_.size();
        dpsize = dportWhiteSet_.size();
    }
    // 这里的输出不是线程安全的
    printf("source filter enable: %d, the set size: %u\npid filter enanle: %d, the set size: %u", saeb, sasize, peb, psize);
    printf("source port filter enable: %d, the set size: %u", speb, spsize);
    printf("des port filter enable: %d, the set size: %u", dpeb, dpsize);
}

void TcpStateConfig::SetConfig() {

}

bool TcpStateConfig::IsPidFilter(uint32_t pid) {
    if (!pid_enable_) {
        return false;
    }
    std::lock_guard<std::mutex> guard(mutex_);
    if (auto findit = pidWhiteSet_.find(pid); findit != pidWhiteSet_.end()) {
        return false;
    }
    return true;
}

bool TcpStateConfig::IsSaddrFilter(unsigned __int128 saddr) {
    if (!saddr_enable_) {
        return false;
    }
    std::lock_guard<std::mutex> guard(mutex_);
    if (auto findit = saddrWhiteSet_.find(saddr); findit != saddrWhiteSet_.end()) {
        return false;
    }
    return true;
}

bool TcpStateConfig::IsSportFilter(uint16_t sport) {
    if (!sport_enable_) {
        return false;
    }
    std::lock_guard<std::mutex> guard(mutex_);
    if (auto findit = sportWhiteSet_.find(sport); findit != sportWhiteSet_.end()) {
        return false;
    }
    return true;
}

bool TcpStateConfig::IsDportFilter(uint16_t dport) {
    if (!dport_enable_) {
        return false;
    }
    std::lock_guard<std::mutex> guard(mutex_);
    if (auto findit = dportWhiteSet_.find(dport); findit != dportWhiteSet_.end()) {
        return false;
    }
    return true;
}


thread_local shptrTcpStateConfig tcp_config;

static void tcpstate_handle_event(void *ctx, int cpu, void *data, __u32 data_sz) {
    char ts[32], saddr[26], daddr[26];
    tcpstate_event *event = reinterpret_cast<tcpstate_event *>(data);

    if (tcp_config->IsPidFilter(event->pid) || tcp_config->IsSaddrFilter(event->saddr)
    || tcp_config->IsSportFilter(event->sport) || tcp_config->IsDportFilter(event->dport) ) {
        return;
    }

    struct tm *tm;
    int family;
    time_t t;

    if (tcp_config->GetEmitTimestamp()) {
        time(&t);
        tm = localtime(&t);
        strftime(ts, sizeof(ts), "%H:%M:%S", tm);
    }
    inet_ntop(event->family, &event->saddr, saddr, sizeof(saddr));
    inet_ntop(event->family, &event->daddr, daddr, sizeof(daddr));
    if (tcp_config->GetWideOutput()) {
        family = event->family == AF_INET ? 4 : 6;
        NANO_LOG(NOTICE, "[%s, %u] %-16llx %-7d %-16s %-2d %-26s %-5d %-26s %-5d %-11s -> %-11s %.3f",
               tcp_config->GetMonitorName().c_str(), tcp_config->GetMonitorId().c_str(), event->skaddr, event->pid, event->task, family, saddr, event->sport, daddr, event->dport,
               tcpstatesMap[event->oldstate].c_str(), tcpstatesMap[event->newstate].c_str(), static_cast<double>(event->delta_us) / 1000);
    } else {
        NANO_LOG(NOTICE, "[%s, %u] %-16llx %-7d %-10.10s %-15s %-5d %-15s %-5d %-11s -> %-11s %.3f",
                 tcp_config->GetMonitorName().c_str(), tcp_config->GetMonitorId().c_str(), event->skaddr, event->pid, event->task, saddr, event->sport, daddr, event->dport,
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
        err = 0;
    }
    cleanup:
        perf_buffer__free(pb);
        tcpstate_bpf__destroy(obj);
    return 0;
}




