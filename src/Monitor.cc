//
// Created by 杨丰硕 on 2022/11/25.
//

#include "Monitor.h"

const std::unordered_map<MonitorType, std::function<int(ring_buffer_sample_fn)>> Monitor::monitorFuncMap = {
    {MonitorType::PROCESS, start_process_monitor},
};

Monitor::Monitor(MonitorType type, uint32_t id):
        monitorId_(id),
        type_(type),
        mainLoop_(monitorFuncMap.at(type)),
        thread_(),
        isRunning_(false) {
}

Monitor::~Monitor() {
    printf("~Monitor\n");
}

void Monitor::start() {
    if (!isRunning_) {
        isRunning_ = true;
        thread_ = std::thread(mainLoop_, nullptr);
        thread_.detach();
    }
}

void Monitor::stop() {
    if (isRunning_) {
        isRunning_ = false;
    }
}

void Monitor::ShowMetadata() const {
    printf("MonitorId: %u; Type: %u; isRunning: %d\n", monitorId_, type_, isRunning_);
}