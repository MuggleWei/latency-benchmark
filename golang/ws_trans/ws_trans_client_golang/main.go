package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"time"

	lc "github.com/MuggleWei/latency-benchmark/golang/latency_common"
	"github.com/gorilla/websocket"
	"gopkg.in/natefinch/lumberjack.v2"
)

func init() {
	log.SetOutput(&lumberjack.Logger{
		Filename:   "./log/ws_trans_client_golang.log",
		MaxSize:    100,   // MB
		MaxBackups: 30,    // old files
		MaxAge:     30,    // day
		Compress:   false, // disabled by default
	})
	// log.SetOutput(os.Stdout)
	log.SetFlags(log.LstdFlags | log.LUTC | log.Lmicroseconds | log.Lshortfile)
}

func run_cts(c *websocket.Conn, config *lc.WsConfig) {
	defer func() {
		c.Close()
	}()

	loop_times := config.Loop
	cnt_per_time := config.CntPerLoop
	loop_interval := config.LoopInterval

	start_ts := time.Now()
	for i := 0; i < loop_times; i++ {
		for j := 0; j < cnt_per_time; j++ {
			ts := time.Now()
			block := lc.TimestampBlock{
				Sec:  ts.Unix(),
				USec: ts.Nanosecond() / 1000,
			}
			b, _ := json.Marshal(block)
			err := c.WriteMessage(websocket.TextMessage, b)
			if err != nil {
				log.Println(err)
				return
			}
		}
		time.Sleep(time.Duration(loop_interval) * time.Millisecond)
	}
	total_elapsed := time.Since(start_ts)
	log.Printf("total elapsed: %v", total_elapsed)

}

func run_stc(c *websocket.Conn, config *lc.WsConfig) {
	maxLen := int64(config.Loop) * int64(config.CntPerLoop)
	elapsed_array := make([]int, maxLen)
	idx := int64(0)

	for {
		_, p, err := c.ReadMessage()
		if err != nil {
			log.Println(err.Error())
			break
		}

		if idx > maxLen-1 {
			break
		}

		ts := time.Now()
		var block lc.TimestampBlock
		json.Unmarshal(p, &block)

		elapsed := int(int64(ts.Nanosecond()/1000-block.USec) + int64((ts.Unix()-block.Sec)*1000000))
		elapsed_array[idx] = elapsed
		idx++
	}

	lc.GenWsReport(elapsed_array[:idx], idx, config)
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

	addr := fmt.Sprintf("ws://%v:%v%v", config.Host, config.Port, config.Url)
	c, _, err := websocket.DefaultDialer.Dial(addr, nil)
	if err != nil {
		log.Printf("[Error] failed dial to %v: %v", addr, err.Error())
		return
	}

	if config.Dir == "cts" {
		run_cts(c, config)
	} else if config.Dir == "stc" {
		run_stc(c, config)
	} else {
		log.Printf("[Error] invalid dir")
	}
}
