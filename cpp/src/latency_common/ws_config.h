#ifndef LATENCY_COMMON_WS_CONFIG_H_
#define LATENCY_COMMON_WS_CONFIG_H_

struct WsConfig
{
	char addr[32];
	char url[128];
	int loop;
	int cnt_per_loop;
	int loop_interval_ms;
	int report_step;
};

bool LoadWsConfig(const char *path, WsConfig *config);

#endif