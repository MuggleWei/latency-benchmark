#include <stdio.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "benchmark_common/benchmark_config.h"

#if MUGGLE_PLATFORM_WINDOWS 
#else
#include <sys/time.h>
#endif

void benchmark_c_time(FILE *fp, muggle::BenchmarkConfig *config)
{
	uint64_t cnt = config->loop * config->cnt_per_loop;
	muggle::LatencyBlock *blocks = (muggle::LatencyBlock*)malloc(cnt * sizeof(muggle::LatencyBlock));

	for (uint64_t i = 0; i < config->loop; ++i)
	{
		for (uint64_t j = 0; j < config->cnt_per_loop; ++j)
		{
			uint64_t idx = i * config->cnt_per_loop + j;
			memset(&blocks[idx], 0, sizeof(muggle::LatencyBlock));
			blocks[idx].idx = idx;

			time_t t;

			timespec_get(&blocks[idx].ts[0], TIME_UTC);
			time(&t);
			timespec_get(&blocks[idx].ts[1], TIME_UTC);
		}

		if (config->loop_interval_ms > 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(config->loop_interval_ms));
		}
	}

	muggle::GenLatencyReportsBody(fp, config, blocks, "c-time", cnt, 0, 1, 0);
	muggle::GenLatencyReportsBody(fp, config, blocks, "c-time-sorted", cnt, 0, 1, 1);
}

void benchmark_c_timespec(FILE *fp, muggle::BenchmarkConfig *config)
{
	uint64_t cnt = config->loop * config->cnt_per_loop;
	muggle::LatencyBlock *blocks = (muggle::LatencyBlock*)malloc(cnt * sizeof(muggle::LatencyBlock));

	for (uint64_t i = 0; i < config->loop; ++i)
	{
		for (uint64_t j = 0; j < config->cnt_per_loop; ++j)
		{
			uint64_t idx = i * config->cnt_per_loop + j;
			memset(&blocks[idx], 0, sizeof(muggle::LatencyBlock));
			blocks[idx].idx = idx;

			struct timespec t;

			timespec_get(&blocks[idx].ts[0], TIME_UTC);
			timespec_get(&t, TIME_UTC);
			timespec_get(&blocks[idx].ts[1], TIME_UTC);
		}

		if (config->loop_interval_ms > 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(config->loop_interval_ms));
		}
	}

	muggle::GenLatencyReportsBody(fp, config, blocks, "c-timespec_get", cnt, 0, 1, 0);
	muggle::GenLatencyReportsBody(fp, config, blocks, "c-timesepc_get-sorted", cnt, 0, 1, 1);
}

void benchmark_gettimeofday(FILE *fp, muggle::BenchmarkConfig *config)
{
	uint64_t cnt = config->loop * config->cnt_per_loop;
	muggle::LatencyBlock *blocks = (muggle::LatencyBlock*)malloc(cnt * sizeof(muggle::LatencyBlock));

	for (uint64_t i = 0; i < config->loop; ++i)
	{
		for (uint64_t j = 0; j < config->cnt_per_loop; ++j)
		{
			uint64_t idx = i * config->cnt_per_loop + j;
			memset(&blocks[idx], 0, sizeof(muggle::LatencyBlock));
			blocks[idx].idx = idx;

			struct timeval t;

			timespec_get(&blocks[idx].ts[0], TIME_UTC);
			gettimeofday(&t, NULL);
			timespec_get(&blocks[idx].ts[1], TIME_UTC);
		}

		if (config->loop_interval_ms > 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(config->loop_interval_ms));
		}
	}

	muggle::GenLatencyReportsBody(fp, config, blocks, "gettimeofday", cnt, 0, 1, 0);
	muggle::GenLatencyReportsBody(fp, config, blocks, "gettimeofday-sorted", cnt, 0, 1, 1);
}

void benchmark_c_gmtime(FILE *fp, muggle::BenchmarkConfig *config)
{
	uint64_t cnt = config->loop * config->cnt_per_loop;
	muggle::LatencyBlock *blocks = (muggle::LatencyBlock*)malloc(cnt * sizeof(muggle::LatencyBlock));

	for (uint64_t i = 0; i < config->loop; ++i)
	{
		for (uint64_t j = 0; j < config->cnt_per_loop; ++j)
		{
			uint64_t idx = i * config->cnt_per_loop + j;
			memset(&blocks[idx], 0, sizeof(muggle::LatencyBlock));
			blocks[idx].idx = idx;

			struct tm buf;
			time_t t = time(NULL);

			timespec_get(&blocks[idx].ts[0], TIME_UTC);
#if MUGGLE_PLATFORM_WINDOWS
			gmtime_s(&t, &buf);
#else
			gmtime_r(&t, &buf);
#endif
			timespec_get(&blocks[idx].ts[1], TIME_UTC);
		}

		if (config->loop_interval_ms > 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(config->loop_interval_ms));
		}
	}

	muggle::GenLatencyReportsBody(fp, config, blocks, "gmtime", cnt, 0, 1, 0);
	muggle::GenLatencyReportsBody(fp, config, blocks, "gmtime-sorted", cnt, 0, 1, 1);
}

int main()
{
	// load config
	muggle::BenchmarkConfig config;
	bool ret = load_benchmark_config("benchmark_conf.json", &config);
	if (!ret)
	{
		return 1;
	}
	strncpy(config.name, "time", sizeof(config.name)-1);

	// create report csv
	char file_name[128];
	snprintf(file_name, sizeof(file_name)-1, "benchmark_%s.csv", config.name);
	FILE *fp = fopen(file_name, "wb");
	if (fp == nullptr)
	{
		printf("failed open file: %s\n", file_name);
		exit(1);
	}

	// write head
	muggle::GenLatencyReportsHead(fp, &config);

	// c time()
	printf("start benchmark: c time\n");
	benchmark_c_time(fp, &config);

	// c timesepc_get()
	printf("start benchmark: c timespec_get\n");
	benchmark_c_timespec(fp, &config);

	// gettimeofday()
	printf("start benchmark: gettimeofday\n");
	benchmark_gettimeofday(fp, &config);

	// c gmtime()
	printf("start benchmark: c gmtime\n");
	benchmark_c_gmtime(fp, &config);

	fclose(fp);

	return 0;
}
