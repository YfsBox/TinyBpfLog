#include <iostream>
#include <thread>
#include <argp.h>
#include <unistd.h>
#include "src/Catalog.h"

bool exit_sig = false;
static void handle_sig(int sig) {
    exit_sig = true;
}

int main(int argc, char **argv) {

    if (getuid()) {
        printf("You should run by root!\n");
        return 0;
    }

    signal(SIGINT, handle_sig);
    signal(SIGTERM, handle_sig);

    Catalog::getInstance().Init("/tmp/logFile");
    while (!exit_sig) {
        printf("TinyBpfTools>");
        std::string cmd;
        std::cin >> cmd;
        if (cmd == "add") {
            printf("Please input the type of your new bpf worker:");
            std::string bpftype;
            std::cin >> bpftype;
            Catalog::getInstance().AddMonitor(bpftype);
        } else if (cmd == "start") {
            printf("Please input the id of moitor your want to run:");
            uint32_t inputId;
            std::cin >> inputId;
            Catalog::getInstance().StartMonitor(inputId);
        } else if (cmd == "stop") {
            printf("Please input the Id you would like to stop:");
            uint32_t inputId;
            std::cin >> inputId;
            Catalog::getInstance().StopAndRemoveMonitor(inputId);
        } else if (cmd == "quit") {
            exit_sig = true;
        } else if (cmd == "show") {
            Catalog::getInstance().ShowMonitors();
        } else {
            printf("Can't exec your cmd!\n");
        }
    }

    Catalog::getInstance().Stop();
    return 0;
}
