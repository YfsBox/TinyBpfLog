//
// Created by 杨丰硕 on 2022/11/25.
//
#include "Catalog.h"
#include "utils.h"
#include "Monitor.h"

void Catalog::ShowMonitors() const {


}

bool Catalog::AddMonitor(const std::string typestr, const std::string logname) {
    if (auto findit = monitorTypeMap_.find(typestr); findit == monitorTypeMap_.end()) {
        printf("The Monitor Type: %s not exist!", typestr.c_str());
        return false;
    }
    Monitor::MonitorType type = monitorTypeMap_[typestr];
    for (auto &id_monitor : monitorMap_) {
        Monitor::MonitorType tp = id_monitor.second->getType();
        std::string log = id_monitor.second->getlogfile();
        if (tp == type) {
            printf("The Monitor Type: %s has conflict!\n", typestr.c_str());
            return false;
        }
        if (log == logname) {
            printf("The Monitor log file: %s is conflict!\n", logname.c_str());
            return false;
        }
    }

    uint32_t monitorid = getRandomNumber();
    monitorMap_[monitorid] = std::make_unique<Monitor>(type, monitorid, logname);

    printf("Create a monitor successfully,the id is %u\n", monitorid);
    return true;
}

bool Catalog::StartMonitor(uint32_t monitorId) {
    auto findit = monitorMap_.find(monitorId);
    if (findit == monitorMap_.end()) {
        printf("The Monitor id: %u not exist!\n", monitorId);
        return false;
    }


}

bool Catalog::StopAndRemoveMonitor(uint32_t monitorId) {
    return true;
}
