package latency_common

type TimestampBlock struct {
	Sec  int64 `json:"s"`
	USec int   `json:"us"`
}
