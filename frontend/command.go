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
		idres, err := DoIdentifier(context, INIT)
		if err != nil {

		}
		fmt.Printf("The id response is %v", idres)
		return nil
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
		idres, err := DoIdentifier(context, RUN)
		if err != nil {

		}
		fmt.Printf("The id response is %v", idres)
		return nil
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
		idres, err := DoIdentifier(context, SET)
		if err != nil {

		}
		fmt.Printf("The id response is %v", idres)
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
		idres, err := DoIdentifier(context, STOP)
		if err != nil {

		}
		fmt.Printf("The id response is %v", idres)
		return nil
	},
}

var LookupCommand = cli.Command{
	Name: LOOKUP,
	Action: func(context *cli.Context) error {
		return nil
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
