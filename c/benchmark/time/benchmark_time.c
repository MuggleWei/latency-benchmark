#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

void func_c_time(void *args, uint64_t idx)
{
	time(NULL);
}

void func_c_clock(void *args, uint64_t idx)
{
	clock();
}

void func_c_timespec(void *args, uint64_t idx)
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
}

void benchmark_func(
	muggle_benchmark_config_t *config,
	fn_muggle_benchmark_func func,
	const char *name)
{
	// initialize benchmark memory pool handle
	muggle_benchmark_func_t benchmark;
	muggle_benchmark_func_init(
		&benchmark,
		config,
		NULL,
		func);

	// run
	muggle_benchmark_func_run(&benchmark);

	// generate report
	muggle_benchmark_func_gen_report(&benchmark, name);

	// destroy benchmark function handle
	muggle_benchmark_func_destroy(&benchmark);
}

int main(int argc, char *argv[])
{
	// initialize log
	muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO);

	// initialize benchmark config
	muggle_benchmark_config_t config;
	muggle_benchmark_config_parse_cli(&config, argc, argv);
	muggle_benchmark_config_output(&config);

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run time");
	benchmark_func(&config, func_c_time, "time");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run clock");
	benchmark_func(&config, func_c_clock, "clock");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run timespec");
	benchmark_func(&config, func_c_timespec, "timespec");
}
