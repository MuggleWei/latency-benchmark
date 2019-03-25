package latency_common

import (
	"encoding/json"
	"io/ioutil"
	"log"
)

type ThreadTransConfig struct {
	Loop         int `json:"loop"`
	CntPerLoop   int `json:"cnt_per_loop"`
	LoopInterval int `json:"loop_interval_ms"`
	ReportStep   int `json:"report_step"`
}

func LoadThreadTransConfig(conf_path string) (*ThreadTransConfig, error) {
	b, err := ioutil.ReadFile(conf_path)
	if err != nil {
		log.Printf("[Error] failed read config file: %v\n", conf_path)
		return nil, err
	}

	var config ThreadTransConfig
	err = json.Unmarshal(b, &config)
	if err != nil {
		log.Printf("[Error] failed parse config file: %v\n", conf_path)
		return nil, err
	}

	return &config, nil
}
