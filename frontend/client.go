package frontend

import (
	"fmt"
	"github.com/urfave/cli"
	"log"
)

type IdentifierResponse struct {
}

func sendIdentifierMsg(name string, id string, action string) (IdentifierResponse, error) {
	idres := IdentifierResponse{}
	return idres, nil
}

func DoIdentifier(context *cli.Context, action_type string) (IdentifierResponse, error) {
	idres := IdentifierResponse{}
	mname, mid, err := getMonitorIdentifier(context)
	if err != nil {
		return idres, fmt.Errorf("getMonitorIdentifier error %v", err)
	}
	log.Printf("The mname is %v and mid is %v\n", mname, mid)
	idres, err = sendIdentifierMsg(mname, mid, action_type)
	if err != nil {
		return idres, fmt.Errorf("SendIdentierMsg error %v", err)
	}
	return idres, nil
}
