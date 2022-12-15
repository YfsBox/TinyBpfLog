package frontend

import (
	"fmt"
	"github.com/urfave/cli"
	"log"
	"net"
	"time"
)

const (
	PROTOCOL_FAMILY = "tcp"
)

type IdentifierResponse struct {
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
		return fmt.Errorf("%v: Create client Conn error", err)
	}
	return nil
}

func (c *Client) sendIdentifierMsg(name string, id string, action string) (IdentifierResponse, error) {
	idres := IdentifierResponse{}
	return idres, nil
}

func (c *Client) stop() error {
	if err := c.Conn.Close(); err != nil {
		return fmt.Errorf("%v: close Conn of client error", err)
	}
	return nil
}

func DoIdentifier(context *cli.Context, action_type string) (IdentifierResponse, error) {
	idres := IdentifierResponse{}
	mname, mid, err := getMonitorIdentifier(context)
	if err != nil {
		return idres, fmt.Errorf("getMonitorIdentifier error %v", err)
	}
	log.Printf("The mname is %v and mid is %v\n", mname, mid)
	idres, err = client.sendIdentifierMsg(mname, mid, action_type)
	if err != nil {
		return idres, fmt.Errorf("SendIdentierMsg error %v", err)
	}
	return idres, nil
}
