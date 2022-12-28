//
// Created by 杨丰硕 on 2022/12/13.
//

#ifndef TINYBPFLOG_SERVER_H
#define TINYBPFLOG_SERVER_H

#include <iostream>

struct MonitorIdentifier {
    bool is_valid_;
    std::string name_;
    std::string id_;
};

class Server {
public:
    friend class ClassForTest;
    static const unsigned VALID_IDMSG_LINENUM = 3;
    static const unsigned VALID_KV_LEN = 2;
    static const int BUFFER_SIZE = 1024;
    static Server& getInstance() {
        static Server server;
        return server;
    }
    int Init(int port);     // create, bind, listen
    void Stop() const;
    void Run();
private:
    void create_fd();
    void listen_fd() const;
    void bind_fd() const;
    Server() = default;
    Server(const Server &server) = delete;
    Server& operator=(const Server &server) = delete;
    int server_fd_;
    int port_;
    char read_buffer[BUFFER_SIZE];
    char write_buffer[BUFFER_SIZE];
};

#endif //TINYBPFLOG_SERVER_H
