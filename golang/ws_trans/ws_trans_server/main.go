package main

import (
	"flag"
	"log"
	"net/http"

	lc "github.com/MuggleWei/latency-benchmark/golang/latency_common"
	"gopkg.in/natefinch/lumberjack.v2"
)

func init() {
	log.SetOutput(&lumberjack.Logger{
		Filename:   "./log/go_ws_trans_server.log",
		MaxSize:    100,   // MB
		MaxBackups: 30,    // old files
		MaxAge:     30,    // day
		Compress:   false, // disabled by default
	})
	// log.SetOutput(os.Stdout)
	log.SetFlags(log.LstdFlags | log.LUTC | log.Lmicroseconds | log.Lshortfile)
}

func main() {
	config_flag := flag.String("config", "ws_conf.json", "config file path")
	flag.Parse()
	config_path := *config_flag

	config, err := lc.LoadWsConfig(config_path)
	if err != nil {
		log.Printf("[Error] failed read %s\n", config_path)
		return
	}

	service := NewWsService(config)

	mux := http.NewServeMux()
	mux.HandleFunc(config.Url, func(w http.ResponseWriter, r *http.Request) {
		service.Hub.OnAccept(w, r)
	})

	server := &http.Server{
		Addr:    config.Addr,
		Handler: mux,
	}
	err = server.ListenAndServe()
	if err != nil {
		log.Fatal("[Fatal] ListenAndServe: %v\n", err)
	}

}
