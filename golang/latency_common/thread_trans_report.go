package latency_common

import (
	"fmt"
	"log"
	"os"
	"sort"
	"strconv"
)

func writeThreadTransReportHead(f *os.File, config *ThreadTransConfig, cnt int64) {
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
	for i := 0; i < 100; i += config.ReportStep {
		s := strconv.Itoa(i)
		f.WriteString(s)
		f.WriteString(",")
	}
	f.WriteString("100")
	f.WriteString("\n")
}

func writeThreadTransReport(f *os.File, config *ThreadTransConfig, sorted bool, elapsed []int, cnt int64) {
	s := fmt.Sprintf("%v,%v,%v,%v,%v,", sorted, config.Loop, config.CntPerLoop, config.LoopInterval, cnt)
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

func GenThreadTransReport(elapsed []int, cnt int64, config *ThreadTransConfig, name string) {
	file_name := "latency-threadtrans-" + name + "-golang.csv"
	f, err := os.Create(file_name)
	if err != nil {
		log.Printf("failed open file: %v\n", file_name)
	}

	defer func() {
		f.Close()
	}()

	writeThreadTransReportHead(f, config, cnt)

	writeThreadTransReport(f, config, false, elapsed, cnt)

	sort.Ints(elapsed)
	writeThreadTransReport(f, config, true, elapsed, cnt)
}
