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

    EXPECT_TRUE(Catalog::getInstance().AddMonitor(PROCESS));
    cft.ShowMonitorConfigs();



}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();
    return 0;
}

