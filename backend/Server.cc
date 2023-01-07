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

#define RUN_CMD "run"
#define INIT_CMD "init"
#define STOP_CMD "stop"
#define SET_CMD "set"

#define NO_ERROR_MSG "null"


static const Monitor *checkExsitMonitor(const MonitorIdentifier &identifier) {
    bool name_empty = identifier.name_.empty();
    bool id_empty = identifier.id_.empty();
    Monitor *monitor = nullptr;
    if (!id_empty && name_empty) {
        std::cout << "get by id " << identifier.id_ << " the id size is " << identifier.id_.size() << "\n";
        monitor = Catalog::getInstance().GetMonitor(identifier.id_);
    } else if (id_empty && !name_empty) {
        std::cout << "get by name\n";
        monitor = Catalog::getInstance().GetMonitorByName(identifier.name_);
    } else {
        std::cout << "get by id " << identifier.id_ << " the id size is " << identifier.id_.size() << "\n";
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
    result.is_valid_ = true;
    if (msglines.size() != Server::VALID_IDMSG_LINENUM) {
        std::cout << "the line size is not valid\n";
        result.is_valid_ = false;
        return result;
    }
    // 划分出kv
    std::vector<std::string> valid_keys = {"msgtype", "m_name", "m_id"};
    size_t curr_key_idx = 0;
    for (auto &line : msglines) {
        auto kvpair = splitString(line, ":");
        if (kvpair.size() != Server::VALID_KV_LEN) {
            std::cout << "the kvpair " << kvpair.size() << " is not valid,the line is " << line << "\n";
            result.is_valid_ = false;
            return result;
        }
        if (kvpair[0] != valid_keys[curr_key_idx]) {  // key不合法
            std::cout << "the kvpair key " << kvpair[0] << " is not valid\n";
            result.is_valid_ = false;
            return result;
        }
        if (curr_key_idx == MonitorIdentifier::NAME_IDX) {
            result.name_ = kvpair[1];
        }
        if (curr_key_idx == MonitorIdentifier::ID_IDX) {
            result.id_ = kvpair[1];
        }
        curr_key_idx++;
    }
    return result;
}

static std::string Identifier2idmsg(const IdResponse &idResponse) {
    std::string is_valid = idResponse.is_valid_?"1\n":"0\n";
    std::string valid_kv = "valid:" + is_valid;
    std::string error_kv = "error:" + idResponse.err_msg_ + "\n";
    std::string state_kv = "state:" + idResponse.state_;
    std::string result = valid_kv + error_kv + state_kv;
    return result;
}

static MessageType getMessageType(const std::vector<std::string> &msg_lines, std::string &header_value) {
    if (msg_lines.size() < 1) {
        return MessageType::ErrType;
    }
    auto header = msg_lines[0];
    auto kv_pair = splitString(header, ":");
    if (kv_pair.size() != Server::VALID_KV_LEN) {
        return MessageType::ErrType;
    }
    header_value = kv_pair[1];
    if (kv_pair[1] == "id") { // 检查value
        return MessageType::IdType;
    } else if (kv_pair[1] == INIT_CMD || kv_pair[1] == SET_CMD || kv_pair[1] == STOP_CMD || kv_pair[1] == RUN_CMD) {
        return MessageType::CmdType;
    } else if (kv_pair[1] == "look") {
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
        printf("The server fd bind error\n");
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
        std::cout << "begin accept a client\n";
        client_fd = ::accept(server_fd_, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);
        std::cout << "get a connect from client, the fd is " << client_fd << '\n';
        if (client_fd < 0) {
            break;
        }
        while (true) {
            char buffer[BUFFER_SIZE];
            bzero(buffer, BUFFER_SIZE);
            size_t len = ::read(client_fd, buffer, BUFFER_SIZE); // 消息会非常短
            if (len <= 0 || len > BUFFER_SIZE) {
                break;
            }
            // 处理逻辑
            std::string msg = buffer;
            std::cout << "The msg is \n" << msg << "read from client\n";
            std::string send_msg; //要发送的字符串
            auto lines = splitString(msg, "\n");
            // 接下来应该有两种类型的消息处理，第一种是id，第二种是执行的cmd
            // 首先判断message类型需要先提取并读取header才可以
            std::string header_value;
            MessageType messageType = getMessageType(lines, header_value);
            // 之后根据header中的type来判断是哪一种类型进一步解析，并构造出相应的response
            switch (messageType) {
                case MessageType::IdType: {
                    MonitorIdentifier midentifier = idmsg2MonitorIdentifier(lines);
                    IdResponse idresponse;
                    if (!midentifier.is_valid_) {
                        std::cout << "The MonitorIdentifier is not valid\n";
                        // 发送不合法的id msg
                        idresponse.is_valid_ = false;
                        idresponse.err_msg_ = "The Id Massenge is not valid format";
                        idresponse.state_ = NO_ERROR_MSG;
                        // 也需要进行发送
                    } else {
                        std::cout << "Server check monitor is exsit\n";
                        const Monitor *findMonitor = checkExsitMonitor(midentifier);
                        std::cout << "find Monitor finish" << '\n';
                        idresponse.is_valid_ = true;
                        idresponse.err_msg_ = NO_ERROR_MSG;
                        if (findMonitor == nullptr) {  // 不存在这个id的情况
                            idresponse.state_ = NOTEXIST_STATE;
                        } else {
                            idresponse.state_ = findMonitor->getStateStr();
                        }
                    }
                    send_msg = Identifier2idmsg(idresponse);
                    break;
                }
                case MessageType::CmdType: {
                    // 首先需要解析出是哪种类型的cmdMessge,从中需要判断合法性,也就是要提取出header type
                    MonitorIdentifier midentifier = idmsg2MonitorIdentifier(lines);
                    const Monitor *findMonitor = checkExsitMonitor(midentifier);
                    IdResponse idResponse;
                    // 之后建立一个swicth,对不同的cmd进行分发
                    bool cmd_error = true;
                    idResponse.err_msg_ = NO_ERROR_MSG;
                    std::cout << "the header value is " << header_value << " ..\n";
                    if (header_value == RUN_CMD) {
                        std::cout << "Start a monitor when run\n";
                        if (cmd_error = Catalog::getInstance().StartMonitor(
                                    findMonitor->getId()); !cmd_error) { // 如果start出错就设置错误信息
                            idResponse.err_msg_ = "Start Monitor " + midentifier.id_ + " error\n";
                        }
                        idResponse.state_ = RUN_STATE;
                    } else if (header_value == SET_CMD) {
                        idResponse.err_msg_ = NO_ERROR_MSG;
                        idResponse.state_ = RUN_STATE;
                        // set header
                    } else if (header_value == STOP_CMD) {
                        std::cout << "run the stop cmd\n";
                        if (cmd_error = Catalog::getInstance().StopAndRemoveMonitor(findMonitor->getId()); !cmd_error) {
                            idResponse.err_msg_ = "Stop Monitor " + midentifier.id_ + " error\n";
                        }
                        idResponse.state_ = STOP_STATE;
                    } else if (header_value == INIT_CMD) {
                        std::cout << "run the init cmd\n";
                        if (cmd_error = Catalog::getInstance().AddMonitor(PROCESS_TYPE,
                                                                          midentifier.name_); !cmd_error) {
                            idResponse.err_msg_ = "Init Monitor " + midentifier.id_ + " error\n";
                        }
                        idResponse.state_ = STOP_STATE;
                    }
                    // 之后再发送response使用，仍然使用IdResponse就好了
                    idResponse.is_valid_ = cmd_error;
                    send_msg = Identifier2idmsg(idResponse);
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
            std::cout << "Server send message is \n" << send_msg << "\n";
            len = ::write(client_fd, send_msg.c_str(), send_msg.size());
            if (len < 0 || len > BUFFER_SIZE) {  // 出现send错误
                break;
            }
        }
        ::close(client_fd); // 关闭链接
    }
}
