package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"runtime"
	"sync/atomic"
	"time"

	lc "github.com/MuggleWei/latency-benchmark/golang/latency_common"
	"golang.org/x/sys/unix"
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
	log.SetOutput(os.Stdout)
	// log.SetFlags(log.LstdFlags | log.LUTC | log.Lmicroseconds | log.Lshortfile)
}

func consumer(
	c chan *lc.LatencyBlock,
	c_signal chan int,
	consumer_ready *int32,
	consumer_read_num []uint64,
) {
	consumer_idx := atomic.AddInt32(consumer_ready, 1)
	c_signal <- int(consumer_idx)
	consumer_idx -= 1

	var idx uint64 = 0
	for {
		block := <-c
		if block == nil {
			break
		}
		block.Ts[2] = time.Now()
		idx++
	}
	consumer_read_num[consumer_idx] = idx

	c_signal <- 2

	switch runtime.GOOS {
	case "linux":
		log.Printf("[%v] consumer complete\n", unix.Gettid())
	}
}

func producer(
	c chan *lc.LatencyBlock,
	c_signal chan int,
	config lc.ThreadTransConfig,
	blocks []lc.LatencyBlock,
	consumer_cnt int,
	start_idx, end_idx uint64,
) {
	for i := 0; i < config.Loop; i++ {
		for j := start_idx; j < end_idx; j++ {
			idx := uint64(i*config.CntPerLoop) + j
			blocks[idx].Idx = idx

			blocks[idx].Ts[0] = time.Now()
			c <- &blocks[idx]
			blocks[idx].Ts[1] = time.Now()
		}
		time.Sleep(time.Millisecond * time.Duration(config.LoopInterval))
	}

	c_signal <- 1

	switch runtime.GOOS {
	case "linux":
		log.Printf("[%v] producer complete\n", unix.Gettid())
	}
}

func benchmark_wr(
	f *os.File,
	config lc.ThreadTransConfig,
	cnt_producer, cnt_consumer int,
) {
	var cnt uint64 = uint64(config.Loop * config.CntPerLoop)
	blocks := make([]lc.LatencyBlock, cnt)
	var consumer_ready int32 = 0

	c := make(chan *lc.LatencyBlock)
	c_signal := make(chan int)

	consumer_read_num := make([]uint64, cnt_consumer)

	for i := 0; i < cnt_consumer; i++ {
		go func() {
			consumer(c, c_signal, &consumer_ready, consumer_read_num)
		}()
	}

	for {
		cnt_consumer_ready := <-c_signal
		if cnt_consumer_ready == cnt_consumer {
			break
		}
	}

	for i := 0; i < cnt_producer; i++ {
		var start_idx uint64 = uint64(i * (config.CntPerLoop / cnt_producer))
		var end_idx uint64 = uint64((i + 1) * (config.CntPerLoop / cnt_producer))
		if i == cnt_producer-1 {
			end_idx = uint64(config.CntPerLoop)
		}

		go func() {
			producer(c, c_signal, config, blocks, cnt_consumer, start_idx, end_idx)
		}()
	}

	// join go routine
	cnt_producer_complete := 0
	cnt_consumer_complete := 0
	for {
		tmp := <-c_signal
		if tmp == 1 {
			cnt_producer_complete += 1
			if cnt_producer_complete == cnt_producer {
				for i := 0; i < cnt_consumer; i++ {
					c <- nil
				}
			}
		} else if tmp == 2 {
			cnt_consumer_complete += 1
			if cnt_consumer_complete == cnt_consumer {
				break
			}
		}
	}

	for i := 0; i < cnt_consumer; i++ {
		log.Printf("%vw%vr consumer[%v] read %v\n",
			cnt_producer, cnt_consumer, i, consumer_read_num[i])
	}

	var s string
	s = fmt.Sprintf("%vw%vr-w", cnt_producer, cnt_consumer)
	lc.GenThreadLantencyReportsBody(f, config, blocks, s, cnt, 0, 1, false)

	s = fmt.Sprintf("%vw%vr-w-sorted", cnt_producer, cnt_consumer)
	lc.GenThreadLantencyReportsBody(f, config, blocks, s, cnt, 0, 1, true)

	s = fmt.Sprintf("%vw%vr-wr", cnt_producer, cnt_consumer)
	lc.GenThreadLantencyReportsBody(f, config, blocks, s, cnt, 0, 2, false)

	s = fmt.Sprintf("%vw%vr-wr-sorted", cnt_producer, cnt_consumer)
	lc.GenThreadLantencyReportsBody(f, config, blocks, s, cnt, 0, 2, true)
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

	file_name := "benchmark_golang_chan.csv"
	f, err := os.Create(file_name)
	if err != nil {
		log.Printf("failed open file: %v\n", file_name)
	}

	defer func() {
		f.Close()
	}()

	lc.GenThreadLantencyReportsHead(f, config)

	// 1 writer, 1 reader
	benchmark_wr(f, *config, 1, 1)

	// cpu number writer, 1 reader
	benchmark_wr(f, *config, runtime.NumCPU(), 1)

	// 2 * cpu number writer, 1 reader
	benchmark_wr(f, *config, 2*runtime.NumCPU(), 1)

	// 1 writer, cpu number reader
	benchmark_wr(f, *config, 1, runtime.NumCPU())

	// 1 writer, 2 * cpu number reader
	benchmark_wr(f, *config, 1, 2*runtime.NumCPU())
}
