package main

import (
	"encoding/json"
	"fmt"
	"log"
	"os"
	"sort"
	"strconv"
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
}

func (this *WsService) OnInactive(peer *cascade.Peer) {
	log.Printf("OnInactive: %v\n", peer.Conn.RemoteAddr().String())

	file_name := "latency-ws-golang.csv"
	f, err := os.Create(file_name)
	if err != nil {
		log.Printf("failed open file: %v\n", file_name)
	}

	defer func() {
		f.Close()
	}()

	this.WriteReportHead(f, this.Config)

	s := this.Elapsed[:this.Idx]
	this.WriteReport(f, this.Config, false, s, this.Idx)
	sort.Ints(s)
	this.WriteReport(f, this.Config, true, s, this.Idx)

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

func (this *WsService) WriteReportHead(f *os.File, config *lc.WsConfig) {
	f.WriteString("sorted")
	f.WriteString(",")
	f.WriteString("loop")
	f.WriteString(",")
	f.WriteString("cnt_per_loop")
	f.WriteString(",")
	f.WriteString("loop_interval_ms")
	f.WriteString(",")
	for i := 0; i < 100; i += config.ReportStep {
		s := strconv.Itoa(i)
		f.WriteString(s)
		f.WriteString(",")
	}
	f.WriteString("100")
	f.WriteString("\n")
}

func (this *WsService) WriteReport(f *os.File, config *lc.WsConfig, sorted bool, elapsed []int, cnt int64) {
	s := fmt.Sprintf("%v,%v,%v,%v,", sorted, config.Loop, config.CntPerLoop, config.LoopInterval)
	f.WriteString(s)
	for i := 0; i < 100; i += config.ReportStep {
		idx := int64((float64(i) / 100.0) * float64(cnt))
		val := fmt.Sprintf("%v,", elapsed[idx])
		f.WriteString(val)
	}
	val := fmt.Sprintf("%v", elapsed[cnt-1])
	f.WriteString(val)
	f.WriteString("\n")
}
