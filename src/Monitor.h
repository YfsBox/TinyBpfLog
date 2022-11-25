//
// Created by 杨丰硕 on 2022/11/25.
//

#ifndef TINYBPFLOG_MONITOR_H
#define TINYBPFLOG_MONITOR_H

#include <functional>
#include <thread>
#include "../bpftools/process_monitor.h"

class Monitor {
public:
    enum class MonitorType {
        PROCESS = 1,
        SYSCALL,
    };

    enum class MonitorStatus {
        START = 0,
        RUNNING,
        STOP,
    };

    Monitor(MonitorType type, uint32_t id, const std::string &logname);
    ~Monitor();

    uint32_t getId() const {
        return monitorId_;
    }

    std::string getlogfile() const {
        return logfile_;
    }

    MonitorType getType() const {
        return type_;
    }

    bool isRunning() const {
        return isRunning_;
    }
    void ShowMetadata() const;
    void start();

private:

    uint32_t monitorId_;
    std::string logfile_;
    // std::function<int(void *ctx, void *data, size_t data_sz)> buf_event_handle_;
    std::function<int(ring_buffer_sample_fn)> mainLoop_;
    std::thread thread_;
    MonitorType type_;
    bool isRunning_;

};


#endif //TINYBPFLOG_MONITOR_H
