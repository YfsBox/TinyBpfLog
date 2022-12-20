package frontend

import (
	"fmt"
	"github.com/urfave/cli"
	"log"
	"net"
	"strings"
	"time"
)

const (
	PROTOCOL_FAMILY = "tcp"
)

// 表示monitor state的枚举类
const (
	notexist_state = iota
	run_state
	stop_state
)

type IdentifierResponse struct {
	IsValid      bool
	MonitorState int
	ErrorMsg     string
}

type CmdResponse struct {
}

type Client struct {
	Addr    string // 表示的是Server的地址
	Conn    net.Conn
	Timeout int64
}

var client Client

func (c *Client) init(addr string, timeout int64) error {
	var err error
	c.Addr = addr
	c.Timeout = timeout
	c.Conn, err = net.DialTimeout(PROTOCOL_FAMILY, c.Addr, 4*time.Second)
	if err != nil {
		return fmt.Errorf("fail when create client Conn error: %v", err)
	}
	return nil
}

func (c *Client) sendMsg(msg string) (string, error) {
	var err error
	_, err = c.Conn.Write([]byte(msg)) // 向server写入msg
	if err != nil {
		return "", fmt.Errorf("fail when client sent msg to server: %v", err)
	}
	var data_len int
	data := make([]byte, 2048)
	data_len, err = c.Conn.Read(data) // 从server获取回应
	if err != nil {
		return "", fmt.Errorf("fail when client get response after send msg: %v", err)
	}
	return string(data[:data_len]), nil
}

func (c *Client) stop() error {
	if err := c.Conn.Close(); err != nil {
		return fmt.Errorf("fail when close Conn of client error: %v", err)
	}
	return nil
}

// 从一个形如"key:value"的字符串中解析出来key和value两部分
func splitKv(msg string) (string, string, error) {
	kv := strings.Split(msg, ":")
	if len(kv) != 2 {
		return "", "", fmt.Errorf("not valid kv")
	}
	return kv[0], kv[1], nil
}

func getIdentifierMsg(mname string, mid string) string {
	msg := fmt.Sprintf("m_name:%v\nm_id:%v", mname, mid)
	return msg
}

func parseIdReponse(resmsg string) (IdentifierResponse, error) {
	var idres IdentifierResponse
	// 首先根据\n来对消息进行分割
	msgs := strings.Split(resmsg, "\n")
	// 判断数量是否合法
	if len(msgs) != 3 { // 只有三个才是合法的
		return idres, fmt.Errorf("the number of message is not valid in response")
	}
	// 逐个检查并构造
	valid_key := [3]string{"valid", "state", "error"}
	valid_value := [3]string{}
	for index, msg := range msgs {
		key, value, err := splitKv(msg)
		if err != nil {
			return idres, fmt.Errorf("not valid kv pair in response")
		}
		if key != valid_key[index] {
			return idres, fmt.Errorf("not valid key in response")
		}
		valid_value[index] = value
	}
	// 处理valid
	switch valid_value[0] {
	case "1":
		idres.IsValid = true
	case "0":
		idres.IsValid = false
	default:
		return idres, fmt.Errorf("the value(valid) is not valid")
	}
	// 处理state
	switch valid_value[1] {
	case "not exist":
		idres.MonitorState = notexist_state
	case "run":
		idres.MonitorState = run_state
	case "stop":
		idres.MonitorState = stop_state
	default:
		return idres, fmt.Errorf("not have this state of monitor")
	}
	// 处理error
	idres.ErrorMsg = valid_value[2]
	return idres, nil
}

// 在该函数中,首先获取monitor_name和monitor_id
// 然后据此生成massenge(字符串的形式)
// 然后调用sendMsg
// 之后解析mag，并将结果整理成结构体作为返回
func DoIdentifier(context *cli.Context) (IdentifierResponse, error) {
	var idres IdentifierResponse
	mname, mid, err := getMonitorIdentifier(context) // 获取monitor_name和monitor_id
	if err != nil {
		return idres, fmt.Errorf("fail when getMonitorIdentifier: %v", err)
	}
	log.Printf("The mname is %v and mid is %v\n", mname, mid)
	msg := getIdentifierMsg(mname, mid)
	response, err := client.sendMsg(msg)
	if err != nil {
		return idres, fmt.Errorf("fail when client send msg in do identifier: %v", err)
	}
	idres, err = parseIdReponse(response)
	if err != nil {
		return idres, fmt.Errorf("fail in parse identifier response: %v", err)
	}
	return idres, nil
}

func SendCmd(context *cli.Context, cmdtype string) (CmdResponse, error) {
	var cmdres CmdResponse

	return cmdres, nil
}

func SendLooKup() error {
	return nil
}
