package frontend

import (
	"fmt"
	"github.com/urfave/cli"
)

const (
	INIT   = "init"
	RUN    = "run"
	SET    = "set"
	STOP   = "stop"
	LOOKUP = "lookup"
)

const (
	name_flag = "n"
	id_flag   = "id"
)

func getMonitorIdentifier(context *cli.Context) (string, string, error) {
	name, id := context.String(name_flag), context.String(id_flag)
	return name, id, nil
}

var InitCommand = cli.Command{
	Name: INIT,
	Flags: []cli.Flag{
		&cli.StringFlag{
			Name:  name_flag,
			Usage: "The name of monitor you want to init",
		},
		&cli.StringFlag{
			Name:  id_flag,
			Usage: "The id of monitor you want to init",
		},
	},
	Action: func(context *cli.Context) error {
		idres, err := DoIdentifier(context)
		if err != nil {
			return err
		}
		fmt.Printf("the id response is %v", idres)
		// 根据返回的response结合Init动作
		if !idres.IsValid {
			return fmt.Errorf("the Identifier response is not valid, the error is %v", idres.ErrorMsg)
		}
		if idres.MonitorState != notexist_state {
			return fmt.Errorf("the Id or Name has exist")
		}
		// 接下来根据发送向该monitor发送cmd指令
		_, err = SendCmd(context, INIT)
		return err
	},
}

var RunCommand = cli.Command{
	Name: RUN,
	Flags: []cli.Flag{
		&cli.StringFlag{
			Name:  name_flag,
			Usage: "The name of monitor you want to run",
		},
		&cli.StringFlag{
			Name:  id_flag,
			Usage: "The id of monitor you want to run",
		},
	},
	Action: func(context *cli.Context) error {
		idres, err := DoIdentifier(context)
		if err != nil {
			return err
		}
		if !idres.IsValid {
			return fmt.Errorf("the Identifier response is not valid, the error is %v", idres.ErrorMsg)
		}
		switch idres.MonitorState {
		case run_state:
			return fmt.Errorf("the monitor is already running")
		case notexist_state:
			return fmt.Errorf("the monitor is not exist")
		}
		_, err = SendCmd(context, RUN)
		return err
	},
}

var SetCommand = cli.Command{
	Name: SET,
	Flags: []cli.Flag{
		&cli.StringFlag{
			Name:  name_flag,
			Usage: "The name of monitor you want to set",
		},
		&cli.StringFlag{
			Name:  id_flag,
			Usage: "The id of monitor you want to set",
		},
	},
	Action: func(context *cli.Context) error {
		idres, err := DoIdentifier(context)
		if err != nil {
			return err
		}
		// fmt.Printf("The id response is %v", idres)
		if !idres.IsValid {
			return fmt.Errorf("the Identifier response is not valid, the error is %v", idres.ErrorMsg)
		}
		if idres.MonitorState == notexist_state {
			return fmt.Errorf("the monitor is not exsit")
		}
		_, err = SendCmd(context, SET)
		return nil
	},
}

var StopCommand = cli.Command{
	Name: STOP,
	Flags: []cli.Flag{
		&cli.StringFlag{
			Name: name_flag,
		},
		&cli.StringFlag{
			Name: id_flag,
		},
	},
	Action: func(context *cli.Context) error {
		idres, err := DoIdentifier(context)
		if err != nil {
			return fmt.Errorf("%v: parse the identifier of monitor error", err)
		}
		// fmt.Printf("The id response is %v", idres)
		if !idres.IsValid {
			return fmt.Errorf("the Identifier response is not valid, the error is %v", idres.ErrorMsg)
		}
		switch idres.MonitorState {
		case notexist_state:
			return fmt.Errorf("the monitor is not exist")
		case stop_state:
			return fmt.Errorf("the monitor is already stop")
		}
		_, err = SendCmd(context, STOP)
		return err
	},
}

var LookupCommand = cli.Command{
	Name: LOOKUP,
	Action: func(context *cli.Context) error {
		err := SendLooKup()
		return err
	},
}

func InitCliApp() *cli.App {
	app := cli.NewApp()
	app.Name = "bpflog frontend"
	app.Usage = "just for fun"

	app.Commands = cli.Commands{
		InitCommand,
		RunCommand,
		SetCommand,
		StopCommand,
		LookupCommand,
	}
	return app
}
