//
// Created by 杨丰硕 on 2022/11/24.
//
#include <memory>
#include "common.h"
#include "monitors.h"
#include "process.h"
#include "process.skel.h"
#include "../runtime/NanoLogCpp17.h"

ProcessConfig::ProcessConfig(uint32_t monitorId, const std::string &monitorName,
                             bool pideb, bool commenab, uint64_t mduration):
    Config(monitorId, monitorName),
    min_duration_(mduration),
    pidenable_(pideb),
    commenable_(commenab) {

}

ProcessConfig::~ProcessConfig() = default;

void ProcessConfig::ShowConfig() {
    uint64_t min_ds = min_duration_;
    bool peb = pidenable_, commeb = commenable_;
    uint32_t psize, commsize;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        psize = pidWhiteSet_.size();
        commsize = commWhiteSet_.size();
    }
    // 这里的输出其实并不是线程安全的
    printf("min duration is %lu\n", min_ds);
    printf("pid enable: %d, pid set size: %u\n", peb, psize);
    printf("comm enable: %d, comm set size: %u\n", commeb, commsize);
}

void ProcessConfig::SetConfig() { // 暂时用来做测试的函数
    char flag;
    printf("If you want to add pid for filter?[y/n]\n");
    std::cin >> flag;
    if (flag == 'y' || flag == 'Y') {
        // 省略去一些关于是否合法的判断
        int pid;
        std::cin >> pid;
        AddPid(pid);
    }
    printf("If you want to add command for filter?[y/n]\n");
    std::cin >> flag;
    if (flag == 'y' || flag == 'Y') {
        std::string command;
        std::cin >> command;
        AddComm(command);
    }
}

void ProcessConfig::AddPid(int pid) {
    if (!pidenable_) {
        pidenable_.store(true);
    }
    std::lock_guard<std::mutex> guard(mutex_);
    pidWhiteSet_.insert(pid);
}

bool ProcessConfig::IsPidFilter(int pid) {
    if (!pidenable_) {
        return false;
    }
    std::lock_guard<std::mutex> guard(mutex_);
    if (auto findit = pidWhiteSet_.find(pid); findit != pidWhiteSet_.end()) {
        return false;
    } // 在名单之中的不被过滤
    return true;
}

void ProcessConfig::AddComm(const std::string &comm) {
    if (!commenable_) {
        commenable_.store(true);
    }
    std::lock_guard<std::mutex> guard(mutex_);
    commWhiteSet_.insert(comm);
}

bool ProcessConfig::IsCommFilter(const std::string &comm) {
    if (!commenable_) {
        return false;
    }
    std::lock_guard<std::mutex> guard(mutex_);
    if (auto findit = commWhiteSet_.find(comm); findit != commWhiteSet_.end()) {
        return false;
    }
    return true;
}

thread_local shptrProcessConfig process_config;
/*std::string monitorName;
std::uint32_t monitorId;*/

int process_handle_event(void *ctx, void *data, size_t data_sz) {
    auto pe = reinterpret_cast<struct process_event*>(data);
    if (process_config->IsPidFilter(pe->pid) || process_config->IsCommFilter(pe->comm)) { // 符合过滤条件
        return 0;
    }

    struct tm *tm;
    char ts[32];
    time_t t;
    time(&t);
    tm = localtime(&t);
    strftime(ts, sizeof(ts), "%H:%M:%S", tm);

    if (pe->exit_event) {
        NANO_LOG(NOTICE, "[%s, %u] %-8s %-5s %-16s %-7d %-7d [%u] (%ums)",
                 process_config->GetMonitorName().c_str(), process_config->GetMonitorId(),
                 ts, "EXIT", pe->comm, pe->pid, pe->ppid, pe->exit_code, pe->duration_ns / 1000000);
    } else {
        NANO_LOG(NOTICE, "[%s, %u] %-8s %-5s %-16s %-7d %-7d %s",
                 process_config->GetMonitorName().c_str(), process_config->GetMonitorId(),
                 ts, "EXEC", pe->comm, pe->pid, pe->ppid, pe->filename);
    }
    return 0;
}

int start_process_monitor(ring_buffer_sample_fn handle_event, const shptrConfig &config) {
    struct ring_buffer *rb = nullptr;
    struct process_bpf *skel;
    int err;
    if (config) {
        process_config = std::dynamic_pointer_cast<ProcessConfig>(config);
        /*monitorId = process_config->GetMonitorId();
        monitorName = process_config->GetMonitorName();*/
    }
    /* Set up libbpf errors and debug info callback */
    libbpf_set_print(libbpf_print_fn);
    /* Bump RLIMIT_MEMLOCK to create BPF maps */
    bump_memlock_rlimit(); // 这些往往在一些简单的demo中都是常规的套路,暂时不需要深究.
    /* Load and verify BPF application */
    skel = process_bpf__open();
    if (!skel) {
        fprintf(stderr, "Failed to open and load BPF skeleton\n");
        return 1;
    }
    /* Parameterize BPF code with minimum duration parameter */
    skel->rodata->min_duration_ns = process_config->GetMinDuration() * 1000000ULL;
    /* Load & verify BPF programs */
    err = process_bpf__load(skel);
    if (err) {
        fprintf(stderr, "Failed to load and verify BPF skeleton\n");
        goto cleanup;
    }

    /* Attach tracepoints */
    err = process_bpf__attach(skel);
    if (err) {
        fprintf(stderr, "Failed to attach BPF skeleton\n");
        goto cleanup;
    }
    /* Set up ring buffer polling */
    // 常见一个ringbuf,作为内核与用户态交互的桥梁.
    rb = ring_buffer__new(bpf_map__fd(skel->maps.rb), process_handle_event, nullptr, nullptr);
    if (!rb) {
        err = -1;
        fprintf(stderr, "Failed to create ring buffer\n");
        goto cleanup;
    }
    /* Process events */
    NANO_LOG(NOTICE, "%-8s %-5s %-16s %-7s %-7s %s\n",
           "TIME", "EVENT", "COMM", "PID", "PPID", "FILENAME/EXIT CODE");
    while (!config->IsExit()) {
        err = ring_buffer__poll(rb, 100 /* timeout, ms */);
        if (err == -EINTR) {
            err = 0;
            break;
        }
        if (err < 0) {
            printf("Error polling perf buffer: %d\n", err);
            break;
        }
    }
    cleanup:
    /* Clean up */
    ring_buffer__free(rb);
    process_bpf__destroy(skel);
    return err < 0 ? -err : 0;
}