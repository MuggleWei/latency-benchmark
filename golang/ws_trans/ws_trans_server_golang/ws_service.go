package main

import (
	"encoding/json"
	"log"
	"os"
	"time"

	lc "github.com/MuggleWei/latency-benchmark/golang/latency_common"

	"github.com/MuggleWei/cascade"
	"github.com/gorilla/websocket"
)

type WsService struct {
	Hub     *cascade.Hub
	Elapsed []int
	Idx     int64
	MaxLen  int64
	Config  *lc.WsConfig
}

func NewWsService(config *lc.WsConfig) *WsService {
	service := &WsService{
		Hub:     nil,
		Elapsed: nil,
		Idx:     0,
		MaxLen:  int64(config.Loop) * int64(config.CntPerLoop),
		Config:  config,
	}
	service.Elapsed = make([]int, service.MaxLen)
	service.Idx = 0

	upgrader := websocket.Upgrader{
		ReadBufferSize:  1024 * 20,
		WriteBufferSize: 1024 * 20,
	}

	hub := cascade.NewHub(service, &upgrader, 10240)
	go hub.Run()

	service.Hub = hub
	return service
}

func (this *WsService) OnActive(peer *cascade.Peer) {
	log.Printf("OnActive: %v\n", peer.Conn.RemoteAddr().String())

	if this.Config.Dir == "stc" {
		c := peer.Conn
		go func() {
			loop_times := this.Config.Loop
			cnt_per_time := this.Config.CntPerLoop
			loop_interval := this.Config.LoopInterval

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

			os.Exit(0)
		}()
	}
}

func (this *WsService) OnInactive(peer *cascade.Peer) {
	log.Printf("OnInactive: %v\n", peer.Conn.RemoteAddr().String())

	if this.Config.Dir == "cts" {
		lc.GenWsReport(this.Elapsed[:this.Idx], this.Idx, this.Config)
	}

	os.Exit(0)
}

func (this *WsService) OnRead(peer *cascade.Peer, message []byte) {
	if this.Idx > this.MaxLen-1 {
		return
	}

	ts := time.Now()
	var block lc.TimestampBlock
	json.Unmarshal(message, &block)

	elapsed := int(int64(ts.Nanosecond()/1000-block.USec) + int64((ts.Unix()-block.Sec)*1000000))
	this.Elapsed[this.Idx] = elapsed
	this.Idx++
}

func (this *WsService) OnHubByteMessage(msg *cascade.HubByteMessage) {
}

func (this *WsService) OnHubObjectMessage(*cascade.HubObjectMessage) {
}
