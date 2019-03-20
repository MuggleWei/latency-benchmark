package latency_common

import (
	"encoding/json"
	"io/ioutil"
	"log"
)

type WsConfig struct {
	Host         string `json:"host"`
	Port         int    `json:"port"`
	Url          string `json:"url"`
	Loop         int    `json:"loop"`
	CntPerLoop   int    `json:"cnt_per_loop"`
	LoopInterval int    `json:"loop_interval_ms"`
	ReportStep   int    `json:"report_step"`
}

func LoadWsConfig(conf_path string) (*WsConfig, error) {
	b, err := ioutil.ReadFile(conf_path)
	if err != nil {
		log.Printf("[Error] failed read config file: %v\n", conf_path)
		return nil, err
	}

	var config WsConfig
	err = json.Unmarshal(b, &config)
	if err != nil {
		log.Printf("[Error] failed parse config file: %v\n", conf_path)
		return nil, err
	}

	return &config, nil
}
