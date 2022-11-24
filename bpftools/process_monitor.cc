//
// Created by 杨丰硕 on 2022/11/24.
//
#include "process_monitor.h"
#include "process.skel.h"
#include "../runtime/NanoLogCpp17.h"

static volatile bool exiting = false;

static void sig_handler(int sig) {
    exiting = true;
}

int process_handle_event(void *ctx, void *data, size_t data_sz) {
    struct event *e = (struct event *) data;
    struct tm *tm;
    char ts[32];
    time_t t;

    time(&t);
    tm = localtime(&t);
    strftime(ts, sizeof(ts), "%H:%M:%S", tm);

    if (e->exit_event) {
        printf("%-8s %-5s %-16s %-7d %-7d [%u]",
                 ts, "EXIT", e->comm, e->pid, e->ppid, e->exit_code);
        if (e->duration_ns) {
            printf(" (%llums)", e->duration_ns / 1000000);
        }
        printf("\n");
    } else {
        printf("%-8s %-5s %-16s %-7d %-7d %s\n",
                 ts, "EXEC", e->comm, e->pid, e->ppid, e->filename);
    }
    return 0;
}

int start_process_tracer(ring_buffer_sample_fn handle_event) {
    struct ring_buffer *rb = NULL;
    struct process_bpf *skel;
    int err;

    /* Set up libbpf errors and debug info callback */
    libbpf_set_print(libbpf_print_fn);
    /* Bump RLIMIT_MEMLOCK to create BPF maps */
    bump_memlock_rlimit(); // 这些往往在一些简单的demo中都是常规的套路,暂时不需要深究.

    /* Cleaner handling of Ctrl-C */
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    /* Load and verify BPF application */
    skel = process_bpf__open();
    if (!skel) {
        fprintf(stderr, "Failed to open and load BPF skeleton\n");
        return 1;
    }

    /* Parameterize BPF code with minimum duration parameter */
    skel->rodata->min_duration_ns = env.min_duration_ms * 1000000ULL;

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
    rb = ring_buffer__new(bpf_map__fd(skel->maps.rb), process_handle_event, NULL, NULL);
    if (!rb) {
        err = -1;
        fprintf(stderr, "Failed to create ring buffer\n");
        goto cleanup;
    }
    /* Process events */
    printf("%-8s %-5s %-16s %-7s %-7s %s\n",
           "TIME", "EVENT", "COMM", "PID", "PPID", "FILENAME/EXIT CODE");
    while (!exiting) {
        err = ring_buffer__poll(rb, 100 /* timeout, ms */);
        if (err == -EINTR) {
            err = 0;
            // printf("-EINTR\n");
            break;
        }
        if (err < 0) {
            printf("Error polling perf buffer: %d\n", err);
            break;
        }
    }

    cleanup:
    printf("have clean up\n");
    /* Clean up */
    ring_buffer__free(rb);
    process_bpf__destroy(skel);

    return err < 0 ? -err : 0;
}