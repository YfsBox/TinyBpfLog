//
// Created by 杨丰硕 on 2022/12/28.
//
#include <iostream>
#include <gtest/gtest.h>
#include "../backend/Server.h"
#include "class_for_test.h"

TEST(SERVER_TEST, SERVER) {
    Server::getInstance().Init(12230);
    Server::getInstance().Run();
    Server::getInstance().Stop();

}


int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();
    return 0;
}