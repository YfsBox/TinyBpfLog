//
// Created by 杨丰硕 on 2022/11/25.
//

#ifndef TINYBPFLOG_MONITORS_H
#define TINYBPFLOG_MONITORS_H

#include <argp.h>
#include <memory>
#include <mutex>
#include "common.h"
#include "process.h"
#include "ipc.h"

class Monitor;
class Config {
public:
    explicit Config(Monitor *monitor = nullptr):
        exit_(false),
        monitor_(monitor) {}
    virtual ~Config() = default;
    virtual bool SetConfig() = 0;

    void SetExit(bool exit) {
        exit_.store(exit);
    }
    bool IsExit() const {
        return exit_;
    }
protected:
    std::atomic<bool> exit_;
    std::mutex mutex_;
    Monitor *monitor_;
};

using shptrConfig = std::shared_ptr<Config>;

class ProcessConfig: public Config {
public:
    explicit ProcessConfig(Monitor *monitor = nullptr);
    ~ProcessConfig() override;
    bool SetConfig() override;
private:
    void ShowConfig() const;
    std::atomic<bool> pidenable_;
    std::atomic<bool> commenable_;
    std::list<int> pidWhiteList_;
    std::list<std::string> commWhiteList_;
};

class MountConfig: public Config {
public:
    explicit MountConfig(Monitor *monitor = nullptr);
    ~MountConfig() override;
    bool SetConfig() override;
private:
    std::atomic<bool> pidenable_;
    std::list<int> pidWhiteList_;
};

using shptrProcessConfig = std::shared_ptr<ProcessConfig>;
using shptrMountConfig = std::shared_ptr<MountConfig>;

int process_handle_event(void *ctx, void *data, size_t data_sz);
int start_process_monitor(ring_buffer_sample_fn handle_event, shptrConfig config);
int idc_handle_event(void *ctx, void *data, size_t data_sz);
int start_ipc_monitor(ring_buffer_sample_fn handle_event);

int start_mount_monitor(ring_buffer_sample_fn handle_event, shptrConfig config)


#endif //TINYBPFLOG_MONITORS_H
