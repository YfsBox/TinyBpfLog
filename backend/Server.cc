//
// Created by 杨丰硕 on 2022/12/13.
//
#include "Server.h"
#include "Catalog.h"
#include <cstring>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>


static Monitor *checkExsitMonitor(const MonitorIdentifier &identifier) {
    bool name_empty = identifier.name_.empty();
    bool id_empty = identifier.id_.empty();
    Monitor *monitor = nullptr;
    if (!id_empty && name_empty) {
        monitor = Catalog::getInstance().GetMonitor(identifier.id_);
    } else if (id_empty && !name_empty) {
        monitor = Catalog::getInstance().GetMonitorByName(identifier.name_);
    } else {
        monitor = Catalog::getInstance().GetMonitor(identifier.id_);
        if (monitor->getName() != identifier.name_) {
            return nullptr;
        }
    }
    return monitor;
}

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
    size_t curr_key_idx = 0;
    for (auto &line : msglines) {
        auto kvpair = splitString(line, ":");
        if (kvpair.size() != Server::VALID_KV_LEN) {
            result.is_valid_ = false;
            return result;
        }
        if (kvpair[curr_key_idx] != valid_keys[curr_key_idx]) {  // key不合法
            result.is_valid_ = false;
            return result;
        }
        if (curr_key_idx == MonitorIdentifier::NAME_IDX) {
            result.name_ = kvpair[MonitorIdentifier::NAME_IDX];
        }
        if (curr_key_idx == MonitorIdentifier::ID_IDX) {
            result.id_ = kvpair[MonitorIdentifier::ID_IDX];
        }
        curr_key_idx++;
    }
    return result;
}

static std::string MonitorIdentifier2idmsg(const IdResponse &idResponse) {
    std::string result;


    return result;
}

static MessageType getMessageType(const std::vector<std::string> &msg_lines) {
    if (msg_lines.size() < 1) {
        return MessageType::ErrType;
    }
    auto header = msg_lines[0];
    auto kv_pair = splitString(header, ":");
    if (kv_pair.size() != Server::VALID_KV_LEN) {
        return MessageType::ErrType;
    }
    if (kv_pair[1] == "id") { // 检查value
        return MessageType::IdType;
    } else if (kv_pair[1] == "cmd") {
        return MessageType::CmdType;
    } else if (kv_pair[1] == "lookup") {
        return MessageType::LookUpType;
    }
    return MessageType::ErrType;
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
        std::string send_msg; //要发送的字符串
        auto lines = splitString(msg, "\n");
        // 接下来应该有两种类型的消息处理，第一种是id，第二种是执行的cmd
        // 首先判断message类型需要先提取并读取header才可以
        MessageType messageType = getMessageType(lines);
        // 之后根据header中的type来判断是哪一种类型进一步解析，并构造出相应的response
        switch (messageType) {
            case MessageType::IdType: {
                MonitorIdentifier midentifier = idmsg2MonitorIdentifier(lines);
                IdResponse idresponse;
                if (!midentifier.is_valid_) {
                    // 发送不合法的id msg
                    idresponse.is_valid_ = false;
                    idresponse.err_msg_ = "The Id Massenge is not valid format";
                    // 也需要进行发送
                } else {
                    Monitor *findMonitor = checkExsitMonitor(midentifier);
                    if (findMonitor == nullptr) {  // 不存在这个id的情况
                        idresponse.is_valid_ = false;
                        idresponse.err_msg_ = "The id of monitor is not exsit";
                        idresponse.state_ = "not exsit";
                        // 也需要进行发送
                    } else {
                        idresponse.is_valid_ = true;
                        idresponse.err_msg_ = "null";
                        idresponse.state_ = findMonitor->getStateStr();
                    }
                }
                send_msg = MonitorIdentifier2idmsg(idresponse);
                break;
            }
            case MessageType::CmdType: {
                break;
            }
            case MessageType::LookUpType: {
                break;
            }
            default: {
                break;
            }
        }
        // 根据构造出来的responae结构体生成相应的字符串，将该字符串发送出去就好了
        len = ::write(client_fd, send_msg.c_str(), BUFFER_SIZE);
        if (len < 0 || len > BUFFER_SIZE) {  // 出现send错误
            break;
        }
    }
}
