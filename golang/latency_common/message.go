package latency_common

type TimestampBlock struct {
	Sec  int64 `json:"s"`
	NSec int   `json:"ns"`
}
