#include "benchmark/benchmark.h"
#include "muggle/c/muggle_c.h"

#if MUGGLE_SUPPORT_FUTEX

class MuggleMutexFixture : public benchmark::Fixture
{
public:
	void SetUp(const benchmark::State &)
	{
		muggle_futex_init(&futex);
		ival = 0;
	}

	void TearDown(const benchmark::State &)
	{}

public:
	int ival;
	muggle_atomic_int futex;
};

BENCHMARK_DEFINE_F(MuggleMutexFixture, LockUnLock)(benchmark::State &state)
{
	for (auto _ : state)
	{
		muggle_futex_lock(&futex);
		ival++;
		muggle_futex_unlock(&futex);
		muggle_futex_wake_one(&futex);
	}
}
BENCHMARK_REGISTER_F(MuggleMutexFixture, LockUnLock)->Threads(1);
BENCHMARK_REGISTER_F(MuggleMutexFixture, LockUnLock)->Threads(2);
BENCHMARK_REGISTER_F(MuggleMutexFixture, LockUnLock)->Threads(4);
BENCHMARK_REGISTER_F(MuggleMutexFixture, LockUnLock)->Threads(8);

#endif

BENCHMARK_MAIN();
