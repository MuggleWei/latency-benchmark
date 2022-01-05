#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

void func_c_snprintf(void *args, uint64_t idx)
{
	char buf[512];
	snprintf(buf, sizeof(buf), "%d", (int)idx);
}

void vsnprintf_fmt(const char *format, ...)
{
	char buf[512];
	va_list args;

	va_start(args, format);
	vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);
}

void func_c_vsnprintf(void *args, uint64_t idx)
{
	vsnprintf_fmt("%d", (int)idx);
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
	MUGGLE_LOG_INFO("run snprintf");
	benchmark_func(&config, func_c_snprintf, "snprintf");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run vsnprintf");
	benchmark_func(&config, func_c_snprintf, "vsnprintf");
}

