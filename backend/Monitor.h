//
// Created by 杨丰硕 on 2022/11/25.
//
#ifndef TINYBPFLOG_MONITOR_H
#define TINYBPFLOG_MONITOR_H

#include <functional>
#include <thread>
#include <memory>
#include <unordered_map>
#include "../bpftools/monitors.h"

enum class MonitorType : unsigned int {
    PROCESS = 1,
    SYSCALL,
    MOUNT,
    TCPSTATE,
};

class Monitor {
public:
    friend class ClassForTest;
    enum class MonitorStatus: unsigned int {
        START = 0,
        RUNNING,
        STOP,
    };
    static const std::unordered_map<MonitorType, std::function<int(ring_buffer_sample_fn, shptrConfig)>> monitorFuncMap;
    Monitor(MonitorType type, uint32_t id, const std::string &name);
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
    void ShowConfig();
    void start();
    void stop();

    void SetConfig() {
        if (config_) {
            config_->SetConfig();
        }
    }

private:
    void InitConfig();

    uint32_t monitorId_;
    MonitorType type_;
    // std::function<int(void *ctx, void *data, size_t data_sz)> buf_event_handle_;
    std::function<int(ring_buffer_sample_fn, shptrConfig)> mainLoop_;
    std::thread thread_;
    bool isRunning_;
    std::shared_ptr<Config> config_;
    std::string name_;
};

#endif //TINYBPFLOG_MONITOR_H
