#include <iostream>
#include <pthread.h>
#include "runtime/NanoLogCpp17.h"
extern "C" {
#include "bpftools/process_tracer.h"
#include "bpftools/process.h"
}



int main() {
    // NanoLog::setLogFile("/tmp/nanologfile");
    /*NANO_LOG(NOTICE, "%-8s %-5s %-16s %-7s %-7s %s\n",
           "TIME", "EVENT", "COMM", "PID", "PPID", "FILENAME/EXIT CODE");*/
    return 0;
}
