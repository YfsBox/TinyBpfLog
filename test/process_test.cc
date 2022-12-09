//
// Created by 杨丰硕 on 2022/12/9.
//
#include <iostream>
#include <gtest/gtest.h>
#include "../backend/Catalog.h"

const std::string PROCESS = "process";

TEST(SINGLE_PROCMONITOR, SETCONFIG) {
    EXPECT_TRUE(Catalog::getInstance().AddMonitor(PROCESS));
    Catalog::getInstance().ShowMonitors();


}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();
    std::cin.get();
    return 0;
}

