package main

import (
	"fmt"
	"log"
	"net"
	"strings"
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

	c.Conn, err = net.DialTimeout(PROTOCOL_FAMILY, c.Addr, time.Duration(c.Timeout)*time.Second)
	// c.Conn, err = net.Dial(PROTOCOL_FAMILY, c.Addr)
	if err != nil {
		return fmt.Errorf("(c *Client)init/%v", err)
	}
	return nil
}

func (c *Client) sendMsg(msg string) (string, error) {
	var err error
	fmt.Printf("write msg %v\n", msg)
	msg = strings.TrimSuffix(msg, "\n")
	_, err = c.Conn.Write([]byte(msg)) // 向server写入msg
	if err != nil {
		return "", fmt.Errorf("(c *Client)sendMsg/%v", err)
	}
	log.Printf("write finish:\n%v", msg)
	var data_len int
	data := make([]byte, 2048)
	data_len, err = c.Conn.Read(data) // 从server获取回应
	fmt.Printf("read data from server ok\n")
	if err != nil {
		return "", fmt.Errorf("(c *Client)init/%v", err)
	}
	tmp_msg := string(data[:data_len])
	tmp_msg = strings.TrimSuffix(tmp_msg, "\n")
	return tmp_msg, nil
}

func (c *Client) stop() error {
	if err := c.Conn.Close(); err != nil {
		return fmt.Errorf("(c *Client)stop/%v", err)
	}
	return nil
}
