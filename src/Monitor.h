//
// Created by 杨丰硕 on 2022/11/25.
//
#ifndef TINYBPFLOG_MONITOR_H
#define TINYBPFLOG_MONITOR_H

#include <functional>
#include <thread>
#include <unordered_map>
#include "../bpftools/monitors.h"

enum class MonitorType {
    PROCESS = 1,
    SYSCALL,
};

class Monitor {
public:

    enum class MonitorStatus {
        START = 0,
        RUNNING,
        STOP,
    };

    static const std::unordered_map<MonitorType, std::function<int(ring_buffer_sample_fn)>> monitorFuncMap;
    Monitor(MonitorType type, uint32_t id);
    ~Monitor();

    uint32_t getId() const {
        return monitorId_;
    }

    MonitorType getType() const {
        return type_;
    }

    bool isRunning() const {
        return isRunning_;
    }
    void ShowMetadata() const;
    void start();
    void stop();

private:

    uint32_t monitorId_;
    MonitorType type_;
    // std::function<int(void *ctx, void *data, size_t data_sz)> buf_event_handle_;
    std::function<int(ring_buffer_sample_fn)> mainLoop_;
    std::thread thread_;
    bool isRunning_;

};

#endif //TINYBPFLOG_MONITOR_H
