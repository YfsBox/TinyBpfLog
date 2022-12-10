//
// Created by 杨丰硕 on 2022/11/26.
//
#include <memory>
#include "common.h"
#include "monitors.h"
#include "mount.h"
#include "mount.skel.h"
#include "../runtime/NanoLogCpp17.h"

MountConfig::MountConfig(uint32_t monitorId, const std::string &monitorName):
        Config(monitorId, monitorName){
}

MountConfig::~MountConfig() = default;

void MountConfig::ShowConfig() {

}

void MountConfig::SetConfig() {

}

thread_local shptrMountConfig mount_config;

#define PERF_BUFFER_PAGES    64

#if !defined(__GLIBC__) || __GLIBC__ < 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ < 32)
const char *strerrorname_np(int errnum) {
    return nullptr;
}
#endif

static const char *flag_names[] = {
        [0] = "MS_RDONLY",      [1] = "MS_NOSUID",    [2] = "MS_NODEV",         [3] = "MS_NOEXEC",      [4] = "MS_SYNCHRONOUS",
        [5] = "MS_REMOUNT",     [6] = "MS_MANDLOCK",  [7] = "MS_DIRSYNC",       [8] = "MS_NOSYMFOLLOW", [9] = "MS_NOATIME",
        [10] = "MS_NODIRATIME", [11] = "MS_BIND",     [12] = "MS_MOVE",         [13] = "MS_REC",        [14] = "MS_VERBOSE",
        [15] = "MS_SILENT",     [16] = "MS_POSIXACL", [17] = "MS_UNBINDABLE",   [18] = "MS_PRIVATE",    [19] = "MS_SLAVE",
        [20] = "MS_SHARED",     [21] = "MS_RELATIME", [22] = "MS_KERNMOUNT",    [23] = "MS_I_VERSION",  [24] = "MS_STRICTATIME",
        [25] = "MS_LAZYTIME",   [26] = "MS_SUBMOUNT", [27] = "MS_NOREMOTELOCK", [28] = "MS_NOSEC",      [29] = "MS_BORN",
        [30] = "MS_ACTIVE",     [31] = "MS_NOUSER",
};
static const int flag_count = sizeof(flag_names) / sizeof(flag_names[0]);

/*static const char argp_program_doc[] =
        "Trace mount and umount syscalls.\n"
        "\n"
        "USAGE: mountsnoop [-h] [-t] [-p PID] [-v]\n"
        "\n"
        "EXAMPLES:\n"
        "    mountsnoop         # trace mount and umount syscalls\n"
        "    mountsnoop -d      # detailed output (one line per column value)\n"
        "    mountsnoop -p 1216 # only trace PID 1216\n";

static const struct argp_option opts[] = {
        { "pid", 'p', "PID", 0, "Process ID to trace" },
        { "timestamp", 't', NULL, 0, "Include timestamp on output" },
        { "detailed", 'd', NULL, 0, "Output result in detail mode" },
        { "verbose", 'v', NULL, 0, "Verbose debug output" },
        { NULL, 'h', NULL, OPTION_HIDDEN, "Show the full help" },
        {},
};*/

static const char *strflags(__u64 flags) {
    static char str[512];
    int i;
    if (!flags) {
        return "0x0";
    }
    str[0] = '\0';
    for (i = 0; i < flag_count; i++) {
        if (!((1 << i) & flags))
            continue;
        if (str[0])
            strcat(str, " | ");
        strcat(str, flag_names[i]);
    }
    return str;
}

static const char *strerrno(int errnum) {
    const char *errstr;
    static char ret[32] = {};

    if (!errnum)
        return "0";
    ret[0] = '\0';
    errstr = strerrorname_np(-errnum);
    if (!errstr) {
        snprintf(ret, sizeof(ret), "%d", errnum);
        return ret;
    }
    snprintf(ret, sizeof(ret), "-%s", errstr);
    return ret;
}

static const char *gen_call(const struct mount_event *me) {
    static char call[10240];

    memset(call, 0, sizeof(call));
    if (me->op ) {
        snprintf(call, sizeof(call), "umount(\"%s\", %s) = %s", me->dest, strflags(me->flags), strerrno(me->ret));
    }
    else {
        snprintf(
                call,
                sizeof(call),
                "mount(\"%s\", \"%s\", \"%s\", %s, \"%s\") = %s",
                me->src,
                me->dest,
                me->fs,
                strflags(me->flags),
                me->data,
                strerrno(me->ret));
    }
    return call;
}

static void mount_handle_event(void *ctx, int cpu, void *data, __u32 data_sz)
{
    const struct mount_event *me = (struct mount_event *)(data);
    struct tm *tm;
    char ts[32];
    time_t t;
    static const char *op_name[] = {
            [MOUNT] = "MOUNT",
            [UMOUNT] = "UMOUNT",
    };
    time(&t);
    tm = localtime(&t);
    strftime(ts, sizeof(ts), "%H:%M:%S ", tm);
    NANO_LOG(NOTICE, "%s COMM[%s] PID[%d] TID[%d] MNT_NS[%u] OP[%s] %s\n", ts,me->comm,
             me->pid, me->tid, me->mnt_ns,
             op_name[me->op],gen_call(me));
    /*
    if (emit_timestamp)
        printf("\n");
    printf("%sPID:    %d\n", indent, me->pid);
    printf("%sTID:    %d\n", indent, me->tid);
    printf("%sCOMM:   %s\n", indent, me->comm);
    printf("%sOP:     %s\n", indent, op_name[me->op]);
    printf("%sRET:    %s\n", indent, strerrno(me->ret));
    printf("%sLAT:    %lldus\n", indent, me->delta / 1000);
    printf("%sMNT_NS: %u\n", indent, me->mnt_ns);
    printf("%sFS:     %s\n", indent, me->fs);
    printf("%sSOURCE: %s\n", indent, me->backend);
    printf("%sTARGET: %s\n", indent, me->dest);
    printf("%sDATA:   %s\n", indent, me->data);
    printf("%sFLAGS:  %s\n", indent, strflags(me->flags));
    printf("\n");*/
}

/*static void handle_lost_events(void *ctx, int cpu, __u64 lost_cnt) {
    warn("lost %llu events on CPU #%d\n", lost_cnt, cpu);
}*/

int start_mount_monitor(ring_buffer_sample_fn handle_event, const shptrConfig &config) {
    // LIBBPF_OPTS(bpf_object_open_opts, open_opts);
    /*static const struct argp argp = {
            .options = opts,
            .parser = parse_arg,
            .doc = argp_program_doc,
    };*/
    perf_buffer *pb = nullptr;
    perf_buffer_opts pb_opts = {};
    struct mount_bpf *obj;
    int err;
    if (config) {
        mount_config = std::dynamic_pointer_cast<MountConfig>(config);
    }
    /*err = argp_parse(&argp, argc, argv, 0, NULL, NULL);
    if (err)
        return err;*/
    libbpf_set_strict_mode(LIBBPF_STRICT_ALL);
    libbpf_set_print(libbpf_print_fn);
    /*err = ensure_core_btf(&open_opts);
    if (err)
    {
        fprintf(stderr, "failed to fetch necessary BTF for CO-RE: %s\n", strerror(-err));
        return 1;
    }*/
    obj = mount_bpf__open();
    if (!obj) {
        warn("failed to open BPF object\n");
        return 1;
    }
    // obj->rodata->target_pid = target_pid;
    err = mount_bpf__load(obj);
    if (err)
    {
        warn("failed to load BPF object: %d\n", err);
        goto cleanup;
    }
    err = mount_bpf__attach(obj);
    if (err)
    {
        warn("failed to attach BPF programs: %d\n", err);
        goto cleanup;
    }
    pb_opts.sample_cb = mount_handle_event;
    pb_opts.lost_cb = handle_lost_events;
    pb = perf_buffer__new(bpf_map__fd(obj->maps.events), 8, &pb_opts);
    if (!pb) {
        err = -errno;
        warn("failed to open perf buffer: %d\n", err);
        goto cleanup;
    }
    // printf("begin poll mount event\n");
    while (!config->IsExit())
    {
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
    mount_bpf__destroy(obj);
    // cleanup_core_btf(&open_opts);
    return err != 0;
}




