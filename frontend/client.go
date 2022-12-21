package frontend

import (
	"fmt"
	"net"
	"time"
)

const (
	PROTOCOL_FAMILY = "tcp"
)

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
