//
// Created by 杨丰硕 on 2022/11/25.
//

#ifndef TINYBPFLOG_UTILS_H
#define TINYBPFLOG_UTILS_H

#include <iostream>
#include <chrono>
#include <random>

uint32_t getRandomNumber() {
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gine (seed);
    return gine();
}

#endif //TINYBPFLOG_UTILS_H
