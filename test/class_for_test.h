//
// Created by 杨丰硕 on 2022/12/9.
//

#ifndef TINYBPFLOG_CLASS_FOR_TEST_H
#define TINYBPFLOG_CLASS_FOR_TEST_H

#include <vector>
#include "../backend/Catalog.h"

class ClassForTest {
public:
    explicit ClassForTest(Catalog &catalog);
    ~ClassForTest();
    void ShowMonitorConfigs();
    std::vector<std::string> GetMonitorIdList() const;
    shptrConfig GetConfig(Monitor *monitor);

    Catalog &catalog_;

};
// 利用C++ RAII的机制管理资源
ClassForTest::ClassForTest(Catalog &catalog):catalog_(catalog) {
    catalog_.Init("/tmp/logFile");
}

ClassForTest::~ClassForTest() {
    catalog_.Stop();
}

void ClassForTest::ShowMonitorConfigs() {
    for (auto &monitor_pair : catalog_.monitorMap_) {
        monitor_pair.second->ShowConfig();
    }
}

std::vector<std::string> ClassForTest::GetMonitorIdList() const {
    std::vector<std::string> result;
    result.reserve(Catalog::MAX_MONITORS_SIZE);
    for (auto &monitor_pair : catalog_.monitorMap_) {
        result.push_back(monitor_pair.first);
    }
    return result;
}

shptrConfig ClassForTest::GetConfig(Monitor *monitor) {
    if (monitor) {
        return monitor->config_;
    }
    return nullptr;
}

#endif //TINYBPFLOG_CLASS_FOR_TEST_H
