package main

import (
	"flag"
	"log"
	"time"

	lc "github.com/MuggleWei/latency-benchmark/golang/latency_common"
	"gopkg.in/natefinch/lumberjack.v2"
)

func init() {
	log.SetOutput(&lumberjack.Logger{
		Filename:   "./log/thread_trans_channel_golang.log",
		MaxSize:    100,   // MB
		MaxBackups: 30,    // old files
		MaxAge:     30,    // day
		Compress:   false, // disabled by default
	})
	// log.SetOutput(os.Stdout)
	log.SetFlags(log.LstdFlags | log.LUTC | log.Lmicroseconds | log.Lshortfile)
}

func main() {
	config_flag := flag.String("config", "thread_trans_conf.json", "config file path")
	flag.Parse()
	config_path := *config_flag

	config, err := lc.LoadThreadTransConfig(config_path)
	if err != nil {
		log.Printf("[Error] failed read %s\n", config_path)
		return
	}

	max_len := int64(config.Loop * config.CntPerLoop)
	elapsed_array := make([]int, max_len)

	c := make(chan time.Time)
	end_channel := make(chan int)

	cnt := int64(0)

	go func() {
		for {
			t := <-c
			cur := time.Now()
			elapsed := cur.Sub(t)
			elapsed_array[cnt] = int(elapsed / time.Microsecond)
			cnt++
			if cnt >= max_len {
				break
			}
		}
		end_channel <- 1
	}()

	for i := 0; i < config.Loop; i++ {
		for j := 0; j < config.CntPerLoop; j++ {
			c <- time.Now()
		}
		time.Sleep(time.Millisecond * time.Duration(config.LoopInterval))
	}

	// wait for complete
	_ = <-end_channel

	lc.GenThreadTransReport(elapsed_array, cnt, config, "channel")
}
