#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

void func_incre(void *args, uint64_t idx)
{
	int *p_var = (int*)args;
	for (int i = 0; i < 1000; i++)
	{
		(*p_var)++;
	}
}

void func_add_equal(void *args, uint64_t idx)
{
	int *p_var = (int*)args;
	for (int i = 0; i < 1000; i++)
	{
		*p_var += 1;
	}
}

void benchmark_func(
	muggle_benchmark_config_t *config,
	fn_muggle_benchmark_func func,
	const char *name)
{
	int var = 0;

	// initialize benchmark memory pool handle
	muggle_benchmark_func_t benchmark;
	muggle_benchmark_func_init(
		&benchmark,
		config,
		(void*)&var,
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
	MUGGLE_LOG_INFO("run i++ 1k times");
	benchmark_func(&config, func_incre, "int_incre_1k");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run i+=1 1k times");
	benchmark_func(&config, func_add_equal, "int_add_equal_1k");

	return 0;
}
