#ifndef LATENCY_COMMON_WS_CONFIG_H_
#define LATENCY_COMMON_WS_CONFIG_H_

struct WsConfig
{
	char host[32];
	int port;
	char url[128];
	int loop;
	int cnt_per_loop;
	int loop_interval_ms;
	int report_step;
	char dir[8];
};

bool LoadWsConfig(const char *path, WsConfig *config);

#endif
