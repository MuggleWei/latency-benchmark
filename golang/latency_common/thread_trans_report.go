package latency_common

import (
	"fmt"
	"os"
	"sort"
	"strconv"
)

func GenThreadLantencyReportsHead(f *os.File, config *ThreadTransConfig) {
	f.WriteString("case_name")
	f.WriteString(",")
	f.WriteString("loop")
	f.WriteString(",")
	f.WriteString("cnt_per_loog")
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

func GenThreadLantencyReportsBody(
	f *os.File,
	config ThreadTransConfig,
	blocks []LatencyBlock,
	caseName string,
	cnt, ts_begin_idx, ts_end_idx uint64,
	need_sort bool,
) {
	elapseds := make([]int, cnt)
	var i uint64
	for i = 0; i < cnt; i++ {
		elapseds[i] = int(blocks[i].Ts[ts_end_idx].Sub(blocks[i].Ts[ts_begin_idx]))
	}

	if need_sort {
		sort.Ints(elapseds)
	}

	s := fmt.Sprintf("%v,%v,%v,%v,", caseName, config.Loop, config.CntPerLoop, config.LoopInterval)
	f.WriteString(s)
	for i := 0; i < 100; i += config.ReportStep {
		idx := int64((float64(i) / 100.0) * float64(cnt))
		val := fmt.Sprintf("%v,", elapseds[idx])
		f.WriteString(val)
	}
	val := fmt.Sprintf("%v", elapseds[cnt-1])
	f.WriteString(val)
	f.WriteString("\n")
}
