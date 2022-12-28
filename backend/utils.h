//
// Created by 杨丰硕 on 2022/11/25.
//

#ifndef TINYBPFLOG_UTILS_H
#define TINYBPFLOG_UTILS_H

#include <iostream>
#include <chrono>
#include <random>

uint32_t getRandomNumber() { // 获取随机数
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gine (seed);
    return gine();
}

std::string getRandomId() {
    return "";
}

#endif //TINYBPFLOG_UTILS_H
