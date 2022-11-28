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

class Catalog {
public:

    using MonitorMap = std::unordered_map<uint32_t, std::unique_ptr<Monitor>>;
    using MonitorTypeMap = std::unordered_map<std::string, MonitorType>;

    static Catalog& getInstance() {
        static Catalog catalog;
        return catalog;
    }
    void Init(const std::string &output);
    void Stop();
    uint32_t ShowMonitors() const;
    bool AddMonitor(const std::string &typestr);
    bool StartMonitor(uint32_t monitorId);
    bool StopAndRemoveMonitor(uint32_t monitorId);

private:
    Catalog() = default;
    Catalog(const Catalog &catalog) = delete;
    Catalog& operator = (const Catalog &catalog) = delete;

    static const uint32_t MAX_MONITORS_SIZE = 20;
    bool isRunning_;
    std::mutex mutex_;
    std::string outputFile_;
    MonitorMap monitorMap_;
    MonitorTypeMap monitorTypeMap_ = {
            {"process", MonitorType::PROCESS},
            {"syscall", MonitorType::SYSCALL},
            {"mount", MonitorType::MOUNT},
    };
};


#endif //TINYBPFLOG_CATALOG_H
