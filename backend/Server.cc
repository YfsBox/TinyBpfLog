//
// Created by 杨丰硕 on 2022/12/13.
//
#include "Server.h"

int Server::Init(int port) {
    port_ = port;
    // 创建socket,listen等操作
    return 0;
}

void Server::Stop() {
    // 关闭socket等等
}
