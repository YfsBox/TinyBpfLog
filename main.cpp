#include <iostream>
#include <thread>
#include <argp.h>
#include <unistd.h>
#include "backend/Catalog.h"
#include "backend/Server.h"



bool exit_sig = false;
static void handle_sig(int sig) {
    exit_sig = true;
}

int main(int argc, char **argv) {
    if (getuid()) {
        printf("You should run by root!\n");
        return 1;
    }
#ifdef RUNNING_DAEMON
    pid_t pid = fork();
    if (pid == - 1) {
        // 关于输出错误信息这方面暂且保留
        return 1;
    }
    if (pid > 0) {
        // 将父进程退出
        exit(0);
    }
    pid = setsid();
    if(pid == -1){
        // 暂且不做关于错误信息的输出
        return 1;
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    Catalog::getInstance().Init("/tmp/logFile");
    Server::getInstance().Init(12234);
    while (true) {
        if (exit_sig) {
            break;
        }
    }
#else
    signal(SIGINT, handle_sig);
    signal(SIGTERM, handle_sig);

    Catalog::getInstance().Init("/tmp/logFile");
    while (!exit_sig) {
        printf("TinyBpfTools>");
        std::string cmd;
        std::cin >> cmd;
        if (cmd == "add") {
            printf("Please input the type of your new bpf worker:");
            std::string bpftype, name;
            std::cin >> bpftype >> name;
            Catalog::getInstance().AddMonitor(bpftype, name);
        } else if (cmd == "start") {
            printf("Please input the id of moitor your want to run:");
            std::string inputId;
            std::cin >> inputId;
            Catalog::getInstance().StartMonitor(inputId);
        } else if (cmd == "stop") {
            printf("Please input the Id you would like to stop:");
            std::string inputId;
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
#endif
    Server::getInstance().Stop();
    Catalog::getInstance().Stop();
    return 0;
}
