#include "benchmark/benchmark.h"
#include "muggle/c/muggle_c.h"

class MuggleAtomicFixture : public benchmark::Fixture
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
	muggle_atomic_int ival;

	muggle_atomic_int inext;
	muggle_atomic_int icursor;
};

// Store
BENCHMARK_DEFINE_F(MuggleAtomicFixture, Store)(benchmark::State &state)
{
	for (auto _ : state)
	{
		muggle_atomic_store(&ival, ival+1, muggle_memory_order_relaxed);
	}
}
BENCHMARK_REGISTER_F(MuggleAtomicFixture, Store)->Threads(1);

// FetchAdd
BENCHMARK_DEFINE_F(MuggleAtomicFixture, FetchAdd)(benchmark::State &state)
{
	for (auto _ : state)
	{
		muggle_atomic_fetch_add(&ival, 1, muggle_memory_order_relaxed);
	}
}
BENCHMARK_REGISTER_F(MuggleAtomicFixture, FetchAdd)->Threads(1);
BENCHMARK_REGISTER_F(MuggleAtomicFixture, FetchAdd)->Threads(2);
BENCHMARK_REGISTER_F(MuggleAtomicFixture, FetchAdd)->Threads(4);
BENCHMARK_REGISTER_F(MuggleAtomicFixture, FetchAdd)->Threads(8);

// CAS
BENCHMARK_DEFINE_F(MuggleAtomicFixture, CAS)(benchmark::State &state)
{
	for (auto _ : state)
	{
		muggle_atomic_int idx = muggle_atomic_fetch_add(&inext, 1, muggle_memory_order_relaxed);
		muggle_atomic_int cur_idx = idx;
		while (!muggle_atomic_cmp_exch_weak(&icursor, &cur_idx, idx + 1, muggle_memory_order_release)
			&& cur_idx != idx)
		{
			muggle_thread_yield();
			cur_idx = idx;
		}
	}
}
BENCHMARK_REGISTER_F(MuggleAtomicFixture, CAS)->Threads(1);
BENCHMARK_REGISTER_F(MuggleAtomicFixture, CAS)->Threads(2);
BENCHMARK_REGISTER_F(MuggleAtomicFixture, CAS)->Threads(4);
BENCHMARK_REGISTER_F(MuggleAtomicFixture, CAS)->Threads(8);

BENCHMARK_MAIN();

