//
// Created by 杨丰硕 on 2022/11/25.
//

#include "Monitor.h"

const std::unordered_map<MonitorType, std::function<int(ring_buffer_sample_fn,
                                                        shptrConfig)>> Monitor::monitorFuncMap = {
        {MonitorType::PROCESS, start_process_monitor},
        {MonitorType::MOUNT,   start_mount_monitor},
};

Monitor::Monitor(MonitorType type, uint32_t id) :
        monitorId_(id),
        type_(type),
        mainLoop_(monitorFuncMap.at(type)),
        thread_(),
        isRunning_(false) {
    InitConfig();
}

Monitor::~Monitor() {
    printf("~Monitor\n");
}

void Monitor::start() {
    if (!isRunning_) {
        config_->SetExit(false);
        thread_ = std::thread(mainLoop_, nullptr, config_);
        thread_.detach();
        isRunning_ = true;
    }
}

void Monitor::stop() {
    if (isRunning_) {
        isRunning_ = false;
        config_->SetExit(true);
    }
}

void Monitor::ShowMetadata() const {
    printf("MonitorId: %u; Type: %u; isRunning: %d\n", monitorId_, type_, isRunning_);
}

void Monitor::InitConfig() {
    switch (type_) {
        case MonitorType::PROCESS: {
            auto config = std::make_shared<ProcessConfig>(monitorId_);
            config_ = std::dynamic_pointer_cast<Config>(config);
            break;
        }
        case MonitorType::SYSCALL: {
            config_ = nullptr; // 先留着
            break;
        }
        case MonitorType::MOUNT: {
            auto config = std::make_shared<MountConfig>(monitorId_);
            config_ = std::dynamic_pointer_cast<Config>(config);
            break;
        }
        default:
            break;
    }
}