//
// Created by 杨丰硕 on 2022/12/13.
//
#include "Server.h"
#include <cstring>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

static std::vector<std::string> splitString(const std::string &str, const std::string &split_flag) {
    std::vector<std::string> result;
    boost::split(result, str, boost::is_any_of(split_flag), boost::token_compress_on);
    return result;
}   // 用来分割字符串*/

static MonitorIdentifier idmsg2MonitorIdentifier(const std::vector<std::string> &msglines) {
    MonitorIdentifier result;
    if (msglines.size() != Server::VALID_IDMSG_LINENUM) {
        result.is_valid_ = false;
        return result;
    }
    // 划分出kv
    std::vector<std::string> valid_keys = {"msgtype", "m_name", "m_id"};
    size_t key_idx = 0;
    for (auto &line : msglines) {
        auto kvpair = splitString(line, ":");
        if (kvpair.size() != Server::VALID_KV_LEN) {
            result.is_valid_ = false;
            return result;
        }
        if (kvpair[key_idx] != valid_keys[key_idx]) {  // key不合法
            result.is_valid_ = false;
            return result;
        }
        if (key_idx == 1) {
            result.name_ = kvpair[1];
        }
        if (key_idx == 2) {
            result.id_ = kvpair[2];
        }
        key_idx++;
    }

    return result;
}


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
        std::string msg = read_buffer;
        auto lines = splitString(msg, "\n");
        MonitorIdentifier midentifier = idmsg2MonitorIdentifier(lines);
        if (!midentifier.is_valid_) {
            // 发送不合法的id msg
            continue; // 不再继续做处理
        }


    }
}
