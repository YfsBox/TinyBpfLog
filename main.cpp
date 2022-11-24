#include <iostream>
#include <thread>
#include "bpftools/process_monitor.h"



int main() {
    //NanoLog::setLogFile("/tmp/nanologfile");
    /*NANO_LOG(NOTICE, "%-8s %-5s %-16s %-7s %-7s %s\n",
           "TIME", "EVENT", "COMM", "PID", "PPID", "FILENAME/EXIT CODE");*/
    std::thread([](){
        start_process_tracer(process_handle_event);
    }).join();

    return 0;
}
