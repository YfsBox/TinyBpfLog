//
// Created by 杨丰硕 on 2022/11/25.
//
#include "monitors.h"
#include "ipc.skel.h"
#include "ipc.h"
#include "../runtime/NanoLogCpp17.h"

static volatile bool exiting = false;

int handle_event(void *ctx, void *data, size_t data_sz) {
    auto e = reinterpret_cast<struct ipc_event *>(data);
    struct tm *tm;
    char ts[32];

    time_t t;
    time(&t);
    tm = localtime(&t);
    strftime(ts, sizeof(ts), "%H:%M:%S", tm);
    NANO_LOG(NOTICE, "%-8s %u %u %u [%u] %u\n",
           ts,  e->pid, e->uid, e->gid, e->cuid, e->cgid);
    return 0;
}

int start_ipc_monitor(ring_buffer_sample_fn handle_event) {
    struct ring_buffer *rb = nullptr;
    struct ipc_bpf *skel;
    int err;
    /* Parse command line arguments */
    libbpf_set_strict_mode(LIBBPF_STRICT_ALL);
    /* Set up libbpf errors and debug info callback */
    libbpf_set_print(libbpf_print_fn);
    bump_memlock_rlimit();

    /* Load and verify BPF application */
    skel = ipc_bpf__open();
    if (!skel) {
        fprintf(stderr, "Failed to open and load BPF skeleton\n");
        return 1;
    }
    /* Load & verify BPF programs */
    err = ipc_bpf__load(skel);
    if (err)
    {
        fprintf(stderr, "Failed to load and verify BPF skeleton\n");
        goto cleanup;
    }
    /* Attach tracepoints */
    err = ipc_bpf__attach(skel);
    if (err)
    {
        fprintf(stderr, "Failed to attach BPF skeleton\n");
        goto cleanup;
    }
    /* Set up ring buffer polling */
    rb = ring_buffer__new(bpf_map__fd(skel->maps.events), handle_event, nullptr, nullptr);
    if (!rb)
    {
        err = -1;
        fprintf(stderr, "Failed to create ring buffer\n");
        goto cleanup;
    }
    /* Process events */
    NANO_LOG(NOTICE, "%-8s %-5s %-16s %-7s %-7s %s\n", "TIME", "PID", "UID", "GID", "CUID", "CGID");
    while (!exiting)
    {
        err = ring_buffer__poll(rb, 100 /* timeout, ms */);
        /* Ctrl-C will cause -EINTR */
        if (err == -EINTR)
        {
            err = 0;
            break;
        }
        if (err < 0)
        {
            printf("Error polling perf buffer: %d\n", err);
            break;
        }
    }
    cleanup:
    /* Clean up */
    ring_buffer__free(rb);
    ipc_bpf__destroy(skel);
    return err < 0 ? -err : 0;

}



