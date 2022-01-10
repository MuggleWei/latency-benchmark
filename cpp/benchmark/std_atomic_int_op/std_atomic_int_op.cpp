#include <atomic>
#include "benchmark/benchmark.h"

static void BM_AtomicFetchAdd(benchmark::State &state)
{
	std::atomic<int> data{0};
	for (auto _ : state)
	{
		data.fetch_add(1, std::memory_order_relaxed);
	}
}

BENCHMARK(BM_AtomicFetchAdd);

BENCHMARK_MAIN();
