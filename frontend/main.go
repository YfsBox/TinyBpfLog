package frontend

import (
	"log"
	"os"
)

func main() {

	app := InitCliApp()
	if err := app.Run(os.Args); err != nil {
		log.Fatalf("Bpflog frontend error: %v", err)
	}

}
