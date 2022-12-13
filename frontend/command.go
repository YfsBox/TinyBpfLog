package frontend

import "github.com/urfave/cli"

const (
	INIT_FLAG   = "init"
	RUN_FLAG    = "run"
	SET_FLAG    = "init"
	STOP_FLAG   = "stop"
	LOOKUP_FLAG = "lookup"
)

type CliApp struct {
}

var InitCommand = cli.Command{
	Name: INIT_FLAG,
}

var RunCommand = cli.Command{
	Name: RUN_FLAG,
}

var SetCommand = cli.Command{
	Name: SET_FLAG,
}

var StopCommand = cli.Command{
	Name: STOP_FLAG,
}

var LookupCommand = cli.Command{
	Name: LOOKUP_FLAG,
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
