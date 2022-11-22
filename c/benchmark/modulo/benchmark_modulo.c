#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

static uint64_t cap = 0;
static uint64_t cap_sub_1 = 0;

void funcmodulo(void *args, uint64_t idx)
{
	*(uint64_t*)args = idx % cap;
}

void funcIDX_IN_POW_OF_2_RING(void *args, uint64_t idx)
{
	*(uint64_t*)args = IDX_IN_POW_OF_2_RING(idx, cap);
}

#define IDX_IN_POW_OF_2_RING_CAP_SUB_1(idx, cap_sub_1) ((idx) & (cap_sub_1))
void funcIDX_IN_POW_OF_2_RING_CAP_SUB_1(void *args, uint64_t idx)
{
	*(uint64_t*)args = IDX_IN_POW_OF_2_RING_CAP_SUB_1(idx, cap_sub_1);
}

void benchmark_func(
	muggle_benchmark_config_t *config,
	fn_muggle_benchmark_func func,
	const char *name)
{
	uint64_t var = 0;

	// initialize benchmark memory pool handle
	muggle_benchmark_func_t benchmark;
	muggle_benchmark_func_init(
		&benchmark,
		config,
		&var,
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

	cap = next_pow_of_2(config.record_per_round);
	cap_sub_1 = cap - 1;

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run modulo");
	benchmark_func(&config, funcmodulo, "modulo");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run IDX_IN_POW_OF_2_RING");
	benchmark_func(&config, funcIDX_IN_POW_OF_2_RING, "IDX_IN_POW_OF_2_RING");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run IDX_IN_POW_OF_2_RING_CAP_SUB_1");
	benchmark_func(&config, funcIDX_IN_POW_OF_2_RING_CAP_SUB_1, "IDX_IN_POW_OF_2_RING_CAP_SUB_1");
}
