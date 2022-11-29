//
// Created by 杨丰硕 on 2022/11/29.
//
#include <memory>
#include "common.h"
#include "monitors.h"
#include "tcpstate.skel.h"
#include "../runtime/NanoLogCpp17.h"

TcpStateConfig::TcpStateConfig(uint32_t monitorId):
        Config(monitorId){
}

TcpStateConfig::~TcpStateConfig() = default;

bool TcpStateConfig::SetConfig() {
    return true;
}



