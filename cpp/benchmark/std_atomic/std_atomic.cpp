#include <atomic>
#include <thread>
#include "benchmark/benchmark.h"

class StdAtomicFixture : public benchmark::Fixture
{
public:
	void SetUp(const benchmark::State &)
	{
		ival = 0;

		inext = 0;
		icursor = 0;
	}

	void TearDown(const benchmark::State &)
	{}

public:
	std::atomic<int> ival{0};

	std::atomic<int> inext{0};
	std::atomic<int> icursor{0};
};

// Store
BENCHMARK_DEFINE_F(StdAtomicFixture, Store)(benchmark::State &state)
{
	for (auto _ : state)
	{
		ival.store(ival + 1, std::memory_order_relaxed);
	}
}
BENCHMARK_REGISTER_F(StdAtomicFixture, Store)->Threads(1);

// FetchAdd
BENCHMARK_DEFINE_F(StdAtomicFixture, FetchAdd)(benchmark::State &state)
{
	for (auto _ : state)
	{
		ival.fetch_add(1, std::memory_order_relaxed);
	}
}
BENCHMARK_REGISTER_F(StdAtomicFixture, FetchAdd)->Threads(1);
BENCHMARK_REGISTER_F(StdAtomicFixture, FetchAdd)->Threads(2);
BENCHMARK_REGISTER_F(StdAtomicFixture, FetchAdd)->Threads(4);
BENCHMARK_REGISTER_F(StdAtomicFixture, FetchAdd)->Threads(8);

// CAS
BENCHMARK_DEFINE_F(StdAtomicFixture, CAS)(benchmark::State &state)
{
	for (auto _ : state)
	{
		int idx = inext.fetch_add(1, std::memory_order_relaxed);
		int cur_idx = idx;
		while (!icursor.compare_exchange_weak(cur_idx, idx+1, std::memory_order_release, std::memory_order_relaxed) && cur_idx != idx)
		{
			std::this_thread::yield();
			cur_idx = idx;
		}
	}
}
BENCHMARK_REGISTER_F(StdAtomicFixture, CAS)->Threads(1);
BENCHMARK_REGISTER_F(StdAtomicFixture, CAS)->Threads(2);
BENCHMARK_REGISTER_F(StdAtomicFixture, CAS)->Threads(4);
BENCHMARK_REGISTER_F(StdAtomicFixture, CAS)->Threads(8);

BENCHMARK_MAIN();
