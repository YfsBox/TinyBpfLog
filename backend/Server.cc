//
// Created by 杨丰硕 on 2022/12/13.
//
#include "Server.h"
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>

void Server::create_fd() {
    int ret = ::socket(AF_INET, SOCK_STREAM, 0);
    if (ret < 0) {
        printf("The server fd create error\n");
    }
    server_fd_ = ret;
}

void Server::bind_fd() const {
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port_);

    int ret = ::bind(server_fd_,
                     reinterpret_cast<const sockaddr*>(&server_addr),
                     sizeof(server_addr));
    if (ret < 0) {
        printf("The server fd bind error");
    }
}

void Server::listen_fd() const {
    int ret = ::listen(server_fd_, 128);
    if (ret < 0) {
        printf("The server listen error");
    }
}

int Server::Init(int port) {
    port_ = port;
    // 清空缓冲区
    memset(read_buffer, 0, sizeof(read_buffer));
    memset(write_buffer, 0, sizeof(write_buffer));
    // 创建socket,listen等操作
    create_fd();
    bind_fd();
    listen_fd();

    return 0;
}

void Server::Stop() const {
    // 关闭socket等等
    ::close(server_fd_);
}

void Server::Run() {
    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd;
    while (true) {
        client_fd = ::accept(server_fd_, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);
        if (client_fd < 0) {
            break;
        }
        size_t len = ::read(client_fd, read_buffer, BUFFER_SIZE); // 消息会非常短
        if (len < 0 || len > BUFFER_SIZE) {
            break;
        }
        // 处理逻辑
        std::cout << "The msg is: \n";
        std::cout << read_buffer << '\n';
    }
}
