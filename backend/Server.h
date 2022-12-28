//
// Created by 杨丰硕 on 2022/12/13.
//

#ifndef TINYBPFLOG_SERVER_H
#define TINYBPFLOG_SERVER_H

class Server {
public:
    friend class ClassForTest;
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
