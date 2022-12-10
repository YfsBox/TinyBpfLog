//
// Created by 杨丰硕 on 2022/11/25.
//
#include "Catalog.h"
#include "utils.h"
#include "Monitor.h"
#include "../runtime/NanoLogCpp17.h"

void Catalog::Init(const std::string &output) {
    isRunning_ = true;
    outputFile_ = output;
    NanoLog::setLogFile(output.c_str());
}

void Catalog::Stop() {
    isRunning_ = false;
}

uint32_t Catalog::ShowMonitors() const {
    printf("Show Monitors as follows:\n");
    for (auto &monitor : monitorMap_) {
        monitor.second->ShowMetadata();
    }
    uint32_t num = monitorMap_.size();
    printf("Have %u at all\n", num);
    return num;
}

bool Catalog::AddMonitor(const std::string &typestr, const std::string &name) {
    if (auto findit = monitorTypeMap_.find(typestr); findit == monitorTypeMap_.end()) {
        printf("The Monitor Type: %s not exist!\n", typestr.c_str());
        return false;
    }
    MonitorType type = monitorTypeMap_[typestr];
    /*for (auto &id_monitor : monitorMap_) {
        MonitorType tp = id_monitor.second->getType();
        if (tp == type) {
            printf("The Monitor Type: %s has conflict!\n", typestr.c_str());
            return false;
        }
    }*/
    uint32_t monitorid = getRandomNumber();
    monitorMap_[monitorid] = std::make_unique<Monitor>(type, monitorid, name);
    printf("Create a monitor successfully,the id is %u\n", monitorid);
    return true;
}

bool Catalog::StartMonitor(uint32_t monitorId) {
    auto findit = monitorMap_.find(monitorId);
    if (findit == monitorMap_.end()) {
        printf("The Monitor id: %u not exist!\n", monitorId);
        return false;
    }
    findit->second->start();
    return true;
}

bool Catalog::StopAndRemoveMonitor(uint32_t monitorId) {
    auto findit = monitorMap_.find(monitorId);
    if (findit == monitorMap_.end()) {
        printf("The Monitor id: %u not exist!\n", monitorId);
        return false;
    }
    findit->second->stop();
    return true;
}

Monitor* Catalog::GetMonitor(uint32_t monitorId) const {
    auto findit = monitorMap_.find(monitorId);
    if (findit != monitorMap_.end()) {
        return findit->second.get();
    }
    return nullptr;
}
