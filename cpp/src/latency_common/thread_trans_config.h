#ifndef LATENCY_COMMON_THREAD_TRANS_CONFIG_H_
#define LATENCY_COMMON_THREAD_TRANS_CONFIG_H_

struct ThreadTransConfig
{
	int loop;
	int cnt_per_loop;
	int loop_interval_ms;
	int report_step;
};

bool LoadThreadTransConfig(const char *path, ThreadTransConfig *config);

#endif
