package frontend

import (
	"fmt"
	"github.com/urfave/cli"
	"log"
	"strings"
)

// 表示monitor state的枚举类
const (
	notexist_state = iota
	run_state
	stop_state
)

const (
	VALID_ARGC_IDRES  = 3
	VALID_ARGC_CMDRES = 2
	VALID_KV          = 2

	isvalid_idx      = 0
	errormsg_idx     = 1
	monitorstate_idx = 2
)

type CommonResponse struct {
	IsValid  bool
	ErrorMsg string
}

type IdentifierResponse struct {
	Common       CommonResponse
	MonitorState int
}

type CmdResponse struct {
	Common CommonResponse
}

type LookupResponse struct {
	Common CommonResponse
}

// 从一个形如"key:value"的字符串中解析出来key和value两部分
func splitKv(msg string) (string, string, error) {
	kv := strings.Split(msg, ":")
	if len(kv) != VALID_KV {
		return "", "", fmt.Errorf("not valid kv")
	}
	return kv[0], kv[1], nil
}

func getIdentifierMsg(mname string, mid string) string {
	msg := fmt.Sprintf("msgtype:id\nm_name:%v\nm_id:%v", mname, mid)
	return msg
}

func getCmdMsg(mname string, mid string, cmdtype string) string {
	return fmt.Sprintf("msgtype:cmd\nm_name:%v\nm_id:%v", mname, mid)
}

func getLookupMsg() string {
	return "msgtype:look"
}

func checkAndgetKv(msgs []string, valid_key []string) ([]string, error) {
	valid_value := []string{}
	for index, msg := range msgs {
		key, value, err := splitKv(msg)
		if err != nil {
			return nil, fmt.Errorf("not valid kv pair in response")
		}
		if key != valid_key[index] {
			return nil, fmt.Errorf("not valid key in response")
		}
		valid_value = append(valid_value, value)
	}
	return valid_value, nil
}

func parseValidAndErrmsg(common *CommonResponse, value []string) error {
	switch value[isvalid_idx] { // 处理isvalid
	case "1":
		common.IsValid = true
	case "0":
		common.IsValid = false
	default:
		return fmt.Errorf("the value(valid) is not valid")
	}
	common.ErrorMsg = value[errormsg_idx]
	return nil
}

func parseIdReponse(resmsg string) (IdentifierResponse, error) {
	var idres IdentifierResponse
	// 首先根据\n来对消息进行分割
	msgs := strings.Split(resmsg, "\n")
	// 判断数量是否合法
	if len(msgs) != VALID_ARGC_IDRES { // 只有三个才是合法的
		return idres, fmt.Errorf("the number of message is not valid in response")
	}
	// 逐个检查并构造
	valid_key := []string{"valid", "error", "state"}
	valid_value, err := checkAndgetKv(msgs, valid_key)
	if err != nil {
		return idres, err
	}
	if err = parseValidAndErrmsg(&idres.Common, valid_value); err != nil {
		return idres, err
	}
	switch valid_value[monitorstate_idx] {
	case "not exist":
		idres.MonitorState = notexist_state
	case "run":
		idres.MonitorState = run_state
	case "stop":
		idres.MonitorState = stop_state
	default:
		return idres, fmt.Errorf("not have this state of monitor")
	}
	return idres, nil
}

func parseCommonResponse(resmsg string) (CommonResponse, error) {
	var cmdres CommonResponse
	msgs := strings.Split(resmsg, "\n")
	if len(msgs) != VALID_ARGC_CMDRES {
		return cmdres, fmt.Errorf("the number of message is not valid in response")
	}
	valid_key := []string{"valid", "error"}
	valid_value, err := checkAndgetKv(msgs, valid_key)
	if err != nil {
		return cmdres, err
	}
	if err = parseValidAndErrmsg(&cmdres, valid_value); err != nil {
		return cmdres, err
	}
	return cmdres, nil
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
	mname, mid, err := getMonitorIdentifier(context)
	if err != nil {
		return cmdres, fmt.Errorf("fail when get the id of monitor: %v", err)
	}
	msg := getCmdMsg(mname, mid, cmdtype)
	response, err := client.sendMsg(msg)
	if err != nil {
		return cmdres, fmt.Errorf("fail when client send msg for cmd: %v", err)
	}
	cmdres.Common, err = parseCommonResponse(response)
	if err != nil {
		return cmdres, fmt.Errorf("fail when client parse common response: %v", err)
	}
	return cmdres, nil
}

func SendLooKup() (LookupResponse, error) {
	var lookres LookupResponse
	msg := getLookupMsg()
	response, err := client.sendMsg(msg)
	if err != nil {
		return lookres, fmt.Errorf("fail when client send msg for cmd: %v", err)
	}
	lookres.Common, err = parseCommonResponse(response)
	if err != nil {
		return lookres, fmt.Errorf("fail when client parse common response: %v", err)
	}
	return lookres, nil
}
