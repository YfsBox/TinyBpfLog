package frontend

import (
	"fmt"
	"log"
	"os"
)

func main() {

	if err := client.init("127.0.0.1:12230", 10); err != nil {
		fmt.Printf("%v: client init error\n", err)
	} // 启动client

	app := InitCliApp() // 解析传入的命令,并且执行对应的回调函数
	if err := app.Run(os.Args); err != nil {
		log.Fatalf("Bpflog frontend error: %v", err)
	}

	if err := client.stop(); err != nil { // 关闭client
		fmt.Printf("%v: client stop error\n", err)
	} // 结束

}
