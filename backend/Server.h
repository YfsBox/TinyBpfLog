//
// Created by 杨丰硕 on 2022/12/13.
//

#ifndef TINYBPFLOG_SERVER_H
#define TINYBPFLOG_SERVER_H

class Server {
public:
    friend class ClassForTest;
    static Server& getInstance() {
        static Server server;
        return server;
    }
    int Init(int port);
    void Stop();
private:
    Server() = default;
    Server(const Server &server) = delete;
    Server& operator = (const Server &server) = delete;

    int port_;
};

#endif //TINYBPFLOG_SERVER_H
