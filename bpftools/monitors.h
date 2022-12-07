//
// Created by 杨丰硕 on 2022/11/25.
//

#ifndef TINYBPFLOG_MONITORS_H
#define TINYBPFLOG_MONITORS_H

#include <argp.h>
#include <memory>
#include <mutex>
#include "common.h"

// class Monitor;
class Config {
public:
    explicit Config(uint32_t monitorId):
        exit_(false),
        monitorId_(monitorId) {}
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
    uint32_t monitorId_;
};

using shptrConfig = std::shared_ptr<Config>;
class ProcessConfig: public Config {
public:
    explicit ProcessConfig(uint32_t monitorId);
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
    explicit MountConfig(uint32_t monitorId);
    ~MountConfig() override;
    bool SetConfig() override;
private:
    std::atomic<bool> pidenable_;
    std::list<int> pidWhiteList_;
};

class TcpStateConfig: public Config {
public:
    explicit TcpStateConfig(uint32_t monitorId);
    ~TcpStateConfig() override;
    bool SetConfig() override;
    bool GetEmitTimestamp() const {
        return emit_timestamp_;
    }
    bool GetWideOutput() const {
        return wide_output_;
    }
private:
    bool emit_timestamp_;
    bool wide_output_;

};

using shptrProcessConfig = std::shared_ptr<ProcessConfig>;
using shptrMountConfig = std::shared_ptr<MountConfig>;
using shptrTcpStateConfig = std::shared_ptr<TcpStateConfig>;

int process_handle_event(void *ctx, void *data, size_t data_sz);
int start_process_monitor(ring_buffer_sample_fn handle_event, const shptrConfig &config);
int idc_handle_event(void *ctx, void *data, size_t data_sz);
int start_ipc_monitor(ring_buffer_sample_fn handle_event);

int start_mount_monitor(ring_buffer_sample_fn handle_event, const shptrConfig &config);
int start_tcpstate_monitor(ring_buffer_sample_fn handle_event, const shptrConfig &config);

#endif //TINYBPFLOG_MONITORS_H
