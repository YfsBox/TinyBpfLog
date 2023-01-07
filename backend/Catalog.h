//
// Created by 杨丰硕 on 2022/11/25.
//

#ifndef TINYBPFLOG_CATALOG_H
#define TINYBPFLOG_CATALOG_H

#include <memory>
#include <mutex>
#include <unordered_map>
#include <thread>
#include "Monitor.h"

#define PROCESS_TYPE "process"
#define SYSCALL_TYPE "syscall"
#define MOUNT_TYPE  "mount"
#define TCPSTATE_TYPE "tcpstate"

class Catalog {
public:

    friend class ClassForTest;
    static const size_t MONITOR_ID_LEN = 16;
    using MonitorMap = std::unordered_map<std::string, std::unique_ptr<Monitor>>;
    using MonitorTypeMap = std::unordered_map<std::string, MonitorType>;

    static Catalog& getInstance() {
        static Catalog catalog;
        return catalog;
    }
    void Init(const std::string &output);
    void Stop();
    uint32_t ShowMonitors() const;
    bool AddMonitor(const std::string &typestr, const std::string &name);
    bool StartMonitor(const std::string &monitorId);
    bool StopAndRemoveMonitor(const std::string &monitorId);
    Monitor *GetMonitor(const std::string &monitorId) const;
    Monitor *GetMonitorByName(const std::string &monitorName) const;

private:
    Catalog() = default;
    Catalog(const Catalog &catalog) = delete;
    Catalog& operator=(const Catalog &catalog) = delete;

    static const uint32_t MAX_MONITORS_SIZE = 20;
    bool isRunning_;
    std::mutex mutex_;
    std::string outputFile_;
    MonitorMap monitorMap_;
    MonitorTypeMap monitorTypeMap_ = {
            {PROCESS_TYPE, MonitorType::PROCESS},
            {SYSCALL_TYPE, MonitorType::SYSCALL},
            {MOUNT_TYPE, MonitorType::MOUNT},
            {TCPSTATE_TYPE, MonitorType::TCPSTATE},
    };
};


#endif //TINYBPFLOG_CATALOG_H
