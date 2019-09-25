package latency_common

import "time"

type TimestampBlock struct {
	Sec  int64 `json:"s"`
	NSec int   `json:"ns"`
}

type LatencyBlock struct {
	Idx uint64
	Ts  [8]time.Time
}
