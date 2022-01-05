#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

void* c_malloc(void *pool, size_t block_size)
{
	return malloc(block_size);
}

void c_free(void *pool, void *data)
{
	free(data);
}

void benchmark_malloc_free(muggle_benchmark_config_t *config, const char *name)
{
	// initialize benchmark memory pool handle
	muggle_benchmark_mempool_t benchmark;
	muggle_benchmark_mempool_init(
		&benchmark,
		config,
		1,
		NULL,
		c_malloc,
		c_free);

	// run
	muggle_benchmark_mempool_run(&benchmark);

	// generate report
	muggle_benchmark_mempool_gen_report(&benchmark, name);

	// destroy benchmark memory pool handle
	muggle_benchmark_mempool_destroy(&benchmark);
}

int main(int argc, char *argv[])
{
	// initialize log
	muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO);

	// initialize benchmark config
	muggle_benchmark_config_t config;
	muggle_benchmark_config_parse_cli(&config, argc, argv);

	config.block_size = 0;

	muggle_benchmark_config_output(&config);

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run malloc free - block size: 128");
	config.block_size = 128;
	benchmark_malloc_free(&config, "malloc_free_128");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run malloc free - block size: 256");
	config.block_size = 256;
	benchmark_malloc_free(&config, "malloc_free_256");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run malloc free - block size: 512");
	config.block_size = 512;
	benchmark_malloc_free(&config, "malloc_free_512");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run malloc free - block size: 1024");
	config.block_size = 1024;
	benchmark_malloc_free(&config, "malloc_free_1024");

	MUGGLE_LOG_INFO("--------------------------------------------------------");
	MUGGLE_LOG_INFO("run malloc free - block size: 4096");
	config.block_size = 4096;
	benchmark_malloc_free(&config, "malloc_free_4096");
	
	return 0;
}
