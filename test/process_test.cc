//
// Created by 杨丰硕 on 2022/12/9.
//
#include <iostream>
#include <gtest/gtest.h>
#include "../backend/Catalog.h"
#include "class_for_test.h"

const std::string PROCESS = "process";

TEST(SINGLE_PROCMONITOR, SETCONFIG) {
    ClassForTest cft(Catalog::getInstance());

    EXPECT_TRUE(Catalog::getInstance().AddMonitor(PROCESS, "proc_monitor1"));
    cft.ShowMonitorConfigs();
    // test for command
    auto monitorIdList = cft.GetMonitorIdList();
    EXPECT_EQ(monitorIdList.size(), 1);
    auto monitorId = monitorIdList[0];
    Monitor *monitor = cft.catalog_.GetMonitor(monitorId);
    EXPECT_TRUE(monitor != nullptr);

    shptrConfig config = cft.GetConfig(monitor);
    shptrProcessConfig procconfig = std::reinterpret_pointer_cast<ProcessConfig>(config);
    EXPECT_TRUE(procconfig != nullptr);
    // start
    EXPECT_TRUE(cft.catalog_.StartMonitor(monitorId));
    sleep(20);
    // do some add pid
    printf("Add some pids to White list\n");
    for (int i = 1000000; i < 1050000; i++) {
        procconfig->AddPid(i);
    }
    for (int i = 1000000; i < 1050000; i++) {
        EXPECT_FALSE(procconfig->IsPidFilter(i));
    }
    cft.ShowMonitorConfigs();
    sleep(20);
    // do some add command
    printf("Add some command to White list\n");
    std::vector<std::string> cmds = {
            "./decompressor",
            "/usr/bin/bash",
            "/usr/bin/grep",
            "/usr/bin/sleep",
    };
    for (auto &cmd : cmds) {
        procconfig->AddComm(cmd);
    }
    for (auto &cmd : cmds) {
        EXPECT_FALSE(procconfig->IsCommFilter(cmd));
    }
    cft.ShowMonitorConfigs();
    sleep(20);
    // start and sleep main thread
    // stop and exit
    EXPECT_TRUE(cft.catalog_.StopAndRemoveMonitor(monitorId));
}

TEST(MULTIPLE_PROCMONITOR, SETCONFIG) {
    const int mnum = 5;
    ClassForTest cft(Catalog::getInstance());
    cft.ShowMonitorConfigs();
    // add 5 monitors
    for (int i = 0; i < mnum; i++) {
        char name[20];
        memset(name, 0, sizeof(name));
        sprintf(name, "proc_monitor_%d", i);
        EXPECT_TRUE(Catalog::getInstance().AddMonitor(PROCESS, name));
    }
    // get monitors
    auto monitorIdList = cft.GetMonitorIdList();
    std::vector<Monitor *> monitors;
    monitors.reserve(mnum);
    for (auto monitorId : monitorIdList) {
        monitors.push_back(cft.catalog_.GetMonitor(monitorId));
    }
    EXPECT_TRUE(monitors.size() == mnum);
    // start monitors
    for (auto monitorId : monitorIdList) {
        EXPECT_TRUE(cft.catalog_.StartMonitor(monitorId));
    }
    cft.ShowMonitorConfigs();
    sleep(20);

}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();
    return 0;
}

