//
// Created by 杨丰硕 on 2022/12/9.
//

#ifndef TINYBPFLOG_CLASS_FOR_TEST_H
#define TINYBPFLOG_CLASS_FOR_TEST_H

#include "../backend/Catalog.h"

class ClassForTest {
public:
    explicit ClassForTest(Catalog &catalog);
    ~ClassForTest() = default;
    void ShowMonitorConfigs();

    Catalog &catalog_;
};

ClassForTest::ClassForTest(Catalog &catalog):catalog_(catalog) {

}

void ClassForTest::ShowMonitorConfigs() {
    for (auto &monitor_pair : catalog_.monitorMap_) {
        monitor_pair.second->ShowConfig();
    }
}

#endif //TINYBPFLOG_CLASS_FOR_TEST_H
