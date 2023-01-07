package main

import (
	"fmt"
	"github.com/urfave/cli"
	"log"
	"strings"
)

// 表示monitor state的枚举类
const (
	notexist_state = iota // 0
	run_state             // 1
	stop_state            // 2
	null_state
)

const (
	VALID_ARGC_IDRES  = 3
	VALID_ARGC_CMDRES = 2
	VALID_KV          = 2

	isvalid_idx      = 0
	errormsg_idx     = 1
	monitorstate_idx = 2

	msg_separator = "\n"
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
		return "", "", fmt.Errorf("splitKv:not valid kv")
	}
	return kv[0], kv[1], nil
}

func getIdentifierMsg(mname string, mid string) string {
	msg := fmt.Sprintf("msgtype:id%vm_name:%v%vm_id:%v", msg_separator, mname, msg_separator, mid)
	return msg
}

func getCmdMsg(mname string, mid string, cmdtype string) string {
	return fmt.Sprintf("msgtype:%v%vm_name:%v%vm_id:%v", cmdtype, msg_separator, mname, msg_separator, mid)
}

func getLookupMsg() string {
	return "msgtype:look"
}

func checkAndgetKv(msgs []string, valid_key []string) ([]string, error) {
	valid_value := []string{}
	for index, msg := range msgs {
		key, value, err := splitKv(msg)
		if err != nil {
			return nil, fmt.Errorf("checkAndgetKv:not valid kv pair in response")
		}
		if key != valid_key[index] {
			return nil, fmt.Errorf("checkAndgetKv:not valid key in response")
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
		return fmt.Errorf("parseValidAndErrmsg:the value(valid) is not valid")
	}
	common.ErrorMsg = value[errormsg_idx]
	return nil
}

func parseIdReponse(resmsg string) (IdentifierResponse, error) {
	var idres IdentifierResponse
	// 首先根据\n来对消息进行分割
	msgs := strings.Split(resmsg, msg_separator)
	// 判断数量是否合法
	if len(msgs) != VALID_ARGC_IDRES { // 只有三个才是合法的
		return idres, fmt.Errorf("parseIdResponse:the number of message is not valid in response")
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
		return idres, fmt.Errorf("parseIdReponse:not have \"%v\" this state of monitor", valid_value[monitorstate_idx])
	}
	return idres, nil
}

func parseCommonResponse(resmsg string) (CommonResponse, error) {
	var cmdres CommonResponse
	msgs := strings.Split(resmsg, msg_separator)
	if len(msgs) != VALID_ARGC_CMDRES {
		return cmdres, fmt.Errorf("parseCommonResponse:the number of message is not valid in response")
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
	defer fmt.Printf("DoIdentifier finish\n")
	var idres IdentifierResponse
	mname, mid, err := getMonitorIdentifier(context) // 获取monitor_name和monitor_id
	if err != nil {
		return idres, fmt.Errorf("DoIdentifier/%v", err)
	}
	log.Printf("The mname is %v and mid is %v\n", mname, mid)
	msg := getIdentifierMsg(mname, mid)
	response, err := client.sendMsg(msg)
	if err != nil {
		return idres, fmt.Errorf("DoIdentifier/%v", err)
	}
	log.Printf("The idresponse is %v\n", response)
	idres, err = parseIdReponse(response)
	if err != nil {
		return idres, fmt.Errorf("DoIdentifier/%v", err)
	}
	return idres, nil
}

func SendCmd(context *cli.Context, cmdtype string) (IdentifierResponse, error) {
	var cmdres IdentifierResponse
	mname, mid, err := getMonitorIdentifier(context)
	if err != nil {
		return cmdres, fmt.Errorf("SendCmd/%v", err)
	}
	msg := getCmdMsg(mname, mid, cmdtype)
	response, err := client.sendMsg(msg)
	if err != nil {
		return cmdres, fmt.Errorf("SendCmd/%v", err)
	}
	log.Printf("The Common Response is %v", response)
	cmdres, err = parseIdReponse(response)
	if err != nil {
		return cmdres, fmt.Errorf("SendCmd/%v", err)
	}
	return cmdres, nil
}

func SendLooKup() (IdentifierResponse, error) {
	var lookres IdentifierResponse
	msg := getLookupMsg()
	response, err := client.sendMsg(msg)
	if err != nil {
		return lookres, fmt.Errorf("SendLookup/%v", err)
	}
	lookres, err = parseIdReponse(response)
	if err != nil {
		return lookres, fmt.Errorf("SendLookup/%v", err)
	}
	return lookres, nil
}
