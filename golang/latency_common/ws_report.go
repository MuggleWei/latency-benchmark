package latency_common

import (
	"fmt"
	"log"
	"os"
	"sort"
	"strconv"
)

func writeWsReportHead(f *os.File, config *WsConfig, cnt int64) {
	f.WriteString("sorted")
	f.WriteString(",")
	f.WriteString("loop")
	f.WriteString(",")
	f.WriteString("cnt_per_loop")
	f.WriteString(",")
	f.WriteString("loop_interval_ms")
	f.WriteString(",")
	f.WriteString("pkg_cnt")
	f.WriteString(",")
	f.WriteString("dir")
	f.WriteString(",")
	for i := 0; i < 100; i += config.ReportStep {
		s := strconv.Itoa(i)
		f.WriteString(s)
		f.WriteString(",")
	}
	f.WriteString("100")
	f.WriteString("\n")
}

func writeWsReport(f *os.File, config *WsConfig, sorted bool, elapsed []int, cnt int64) {
	s := fmt.Sprintf("%v,%v,%v,%v,%v,%v,", sorted, config.Loop, config.CntPerLoop, config.LoopInterval, cnt, config.Dir)
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

func GenWsReport(elapsed []int, cnt int64, config *WsConfig) {
	file_name := "latency-ws-" + config.Dir + "-golang.csv"
	f, err := os.Create(file_name)
	if err != nil {
		log.Printf("failed open file: %v\n", file_name)
	}

	defer func() {
		f.Close()
	}()

	writeWsReportHead(f, config, cnt)

	writeWsReport(f, config, false, elapsed, cnt)

	sort.Ints(elapsed)
	writeWsReport(f, config, true, elapsed, cnt)
}
