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
    using MonitorTypeMap = std::unordered_map<std::string, Monitor::MonitorType>;

    static Catalog& getInstance() {
        static Catalog catalog;
        return catalog;
    }

    void ShowMonitors() const;
    bool AddMonitor(const std::string typestr, const std::string logname);
    bool StartMonitor(uint32_t monitorId);
    bool StopAndRemoveMonitor(uint32_t monitorId);

private:
    Catalog() = default;
    Catalog(const Catalog &catalog) = delete;
    Catalog& operator = (const Catalog &catalog) = delete;

    static const uint32_t MAX_MONITORS_SIZE = 20;
    std::mutex mutex_;
    MonitorMap monitorMap_;
    MonitorTypeMap monitorTypeMap_ = {
            {"process", Monitor::MonitorType::PROCESS},
            {"syscall", Monitor::MonitorType::SYSCALL},
    };
};


#endif //TINYBPFLOG_CATALOG_H
