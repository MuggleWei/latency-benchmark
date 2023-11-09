#include <atomic>
#include <thread>
#include "benchmark/benchmark.h"

static thread_local int s_ival = 0;
static thread_local std::atomic<int> s_thread_local_ival{ 0 };

class StdAtomicFixture : public benchmark::Fixture {
public:
	void SetUp(const benchmark::State &)
	{
		s_ival = 0;
		s_thread_local_ival = 0;
		ival = 0;
	}

	void TearDown(const benchmark::State &)
	{
	}

public:
	std::atomic<int> ival{ 0 };

	std::atomic<int> inext{ 0 };
	std::atomic<int> icursor{ 0 };
};

// store
BENCHMARK_DEFINE_F(StdAtomicFixture, Store)(benchmark::State &state)
{
	for (auto _ : state) {
		ival.store(s_ival++, std::memory_order_relaxed);
	}
}
BENCHMARK_REGISTER_F(StdAtomicFixture, Store)->Threads(1);
BENCHMARK_REGISTER_F(StdAtomicFixture, Store)->Threads(2);
BENCHMARK_REGISTER_F(StdAtomicFixture, Store)->Threads(4);
BENCHMARK_REGISTER_F(StdAtomicFixture, Store)->Threads(6);
BENCHMARK_REGISTER_F(StdAtomicFixture, Store)->Threads(8);

// store thread local
BENCHMARK_DEFINE_F(StdAtomicFixture, StoreThreadLocal)(benchmark::State &state)
{
	for (auto _ : state) {
		s_thread_local_ival.store(s_ival++, std::memory_order_relaxed);
	}
}
BENCHMARK_REGISTER_F(StdAtomicFixture, StoreThreadLocal)->Threads(1);
BENCHMARK_REGISTER_F(StdAtomicFixture, StoreThreadLocal)->Threads(2);
BENCHMARK_REGISTER_F(StdAtomicFixture, StoreThreadLocal)->Threads(4);
BENCHMARK_REGISTER_F(StdAtomicFixture, StoreThreadLocal)->Threads(6);
BENCHMARK_REGISTER_F(StdAtomicFixture, StoreThreadLocal)->Threads(8);

// fetchadd
BENCHMARK_DEFINE_F(StdAtomicFixture, FetchAdd)(benchmark::State &state)
{
	for (auto _ : state) {
		ival.fetch_add(1, std::memory_order_relaxed);
	}
}
BENCHMARK_REGISTER_F(StdAtomicFixture, FetchAdd)->Threads(1);
BENCHMARK_REGISTER_F(StdAtomicFixture, FetchAdd)->Threads(2);
BENCHMARK_REGISTER_F(StdAtomicFixture, FetchAdd)->Threads(4);
BENCHMARK_REGISTER_F(StdAtomicFixture, FetchAdd)->Threads(6);
BENCHMARK_REGISTER_F(StdAtomicFixture, FetchAdd)->Threads(8);

// fetchadd thread local
BENCHMARK_DEFINE_F(StdAtomicFixture, FetchAddThreadLocal)
(benchmark::State &state)
{
	for (auto _ : state) {
		s_thread_local_ival.fetch_add(1, std::memory_order_relaxed);
	}
}
BENCHMARK_REGISTER_F(StdAtomicFixture, FetchAddThreadLocal)->Threads(1);
BENCHMARK_REGISTER_F(StdAtomicFixture, FetchAddThreadLocal)->Threads(2);
BENCHMARK_REGISTER_F(StdAtomicFixture, FetchAddThreadLocal)->Threads(4);
BENCHMARK_REGISTER_F(StdAtomicFixture, FetchAddThreadLocal)->Threads(6);
BENCHMARK_REGISTER_F(StdAtomicFixture, FetchAddThreadLocal)->Threads(8);

// CAS
BENCHMARK_DEFINE_F(StdAtomicFixture, CAS)(benchmark::State &state)
{
	for (auto _ : state) {
		int idx = inext.fetch_add(1, std::memory_order_relaxed);
		int cur_idx = idx;
		while (!icursor.compare_exchange_weak(cur_idx, idx + 1,
											  std::memory_order_release,
											  std::memory_order_relaxed) &&
			   cur_idx != idx) {
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
