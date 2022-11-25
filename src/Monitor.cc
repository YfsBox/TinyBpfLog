//
// Created by 杨丰硕 on 2022/11/25.
//

#include "Monitor.h"

Monitor::Monitor(MonitorType type, uint32_t id, const std::string &logname):
        monitorId_(id),
        logfile_(logname),
        mainLoop_(start_process_monitor),
        thread_(mainLoop_, process_handle_event),
        type_(type),
        isRunning_(false) {

}

Monitor::~Monitor() {

}

void Monitor::start() {
    thread_.detach();
}