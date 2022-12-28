//
// Created by 杨丰硕 on 2022/11/25.
//

#ifndef TINYBPFLOG_MONITORS_H
#define TINYBPFLOG_MONITORS_H

#include <argp.h>
#include <memory>
#include <mutex>
#include <set>
#include "common.h"

using Enable = std::atomic<bool>;
// class Monitor;
class Config {
public:
    explicit Config(const std::string &monitorId, const std::string &monitorName):
        exit_(false),
        monitorId_(monitorId),
        monitorName_(monitorName) {

    }
    Config(const Config &config) = delete;
    Config& operator=(const Config &config) = delete;

    virtual ~Config() = default;
    virtual void ShowConfig() = 0;
    void SetExit(bool exit) {
        exit_.store(exit);
    }
    bool IsExit() const {
        return exit_;
    }
    std::string GetMonitorId() const {
        return monitorId_;
    }
    std::string GetMonitorName() const {
        return monitorName_;
    }

    virtual void SetConfig() = 0; // 暂时用来测试的版本SetConfig
protected:
    Enable exit_;
    std::mutex mutex_;
    std::string monitorId_;
    std::string monitorName_;
};

using shptrConfig = std::shared_ptr<Config>;
class ProcessConfig: public Config {
public:
    explicit ProcessConfig(const std::string &monitorId,
                           const std::string &monitorName,
                           bool pideb = false,
                           bool commenab = false,
                           uint64_t mind = 0);
    ~ProcessConfig() override;
    void AddPid(int pid);
    bool IsPidFilter(int pid);

    void AddComm(const std::string &comm);
    bool IsCommFilter(const std::string &comm);
    void ShowConfig() override;

    void SetMinDuration(uint64_t mduration) {
        min_duration_.store(mduration);
    }
    bool IsDutationFilter(uint64_t duration) const {
        return duration < min_duration_;
    }
    uint64_t GetMinDuration() const {
        return min_duration_;
    }
    void SetConfig() override;

private:
    std::atomic<uint64_t> min_duration_; // 这个地方的单位是second不是ns
    Enable pidenable_;
    Enable commenable_;
    std::set<int> pidWhiteSet_;
    std::set<std::string> commWhiteSet_;
};

class MountConfig: public Config {
public:
    explicit MountConfig(const std::string &monitorId,
                         const std::string &monitorName,
                         bool pidenable = false,
                         bool commenable = false,
                         bool destenable = false,
                         bool srcenable = false);
    ~MountConfig() override;
    void ShowConfig() override;
    void SetConfig() override;

    void AddPid(unsigned int pid);
    void AddComm(const std::string &comm);
    void AddDest(const std::string &dest);
    void AddSrc(const std::string &src);

    bool IsPidFilter(unsigned int pid);
    bool IsCommFilter(const std::string &comm);
    bool IsDestFilter(const std::string &dest);
    bool IsSrcFilter(const std::string &src);

private:
    Enable pid_enable_;
    Enable comm_enable_;
    Enable dest_enable_;
    Enable src_enable_;
    // std::list<int> pidWhiteList_;
    std::set<uint16_t> pidWhiteSet_;
    std::set<std::string> commWhiteSet_;
    std::set<std::string> destWhiteSet_;
    std::set<std::string> srcWhiteSet_;
};

class TcpStateConfig: public Config {
public:
    explicit TcpStateConfig(const std::string &monitorId,
                            const std::string &monitorName,
                            bool emit_timestamp = false,
                            bool wide_output = false,
                            bool saddr_eb = false,
                            bool pid_eb = false,
                            bool sport_eb = false,
                            bool dport_eb = false);
    ~TcpStateConfig() override;
    bool GetEmitTimestamp() const {
        return emit_timestamp_;
    }
    bool GetWideOutput() const {
        return wide_output_;
    }
    void ShowConfig() override;

    void AddSaddr(unsigned __int128 saddr);
    void AddPid(uint32_t pid);
    void AddSport(uint16_t sport);
    void AddDport(uint16_t dport);

    bool IsSaddrFilter(unsigned __int128 saddr);
    bool IsPidFilter(uint32_t pid);
    bool IsSportFilter(uint16_t sport);
    bool IsDportFilter(uint16_t dport);

    void SetConfig() override;

private:
    bool emit_timestamp_;
    bool wide_output_;
    Enable saddr_enable_;
    Enable pid_enable_;
    Enable sport_enable_;
    Enable dport_enable_;

    std::set<unsigned __int128> saddrWhiteSet_;
    std::set<uint32_t> pidWhiteSet_;
    std::set<uint16_t> sportWhiteSet_;
    std::set<uint16_t> dportWhiteSet_;
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
