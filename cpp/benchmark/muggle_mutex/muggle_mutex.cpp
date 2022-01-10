#include "benchmark/benchmark.h"
#include "muggle/c/muggle_c.h"

class MuggleMutexFixture : public benchmark::Fixture
{
public:
	void SetUp(const benchmark::State &)
	{
		ival = 0;
	}

	void TearDown(const benchmark::State &)
	{}

public:
	int ival;
	muggle_mutex_t mtx;
};

BENCHMARK_DEFINE_F(MuggleMutexFixture, LockUnLock)(benchmark::State &state)
{
	for (auto _ : state)
	{
		muggle_mutex_lock(&mtx);
		ival++;
		muggle_mutex_unlock(&mtx);
	}
}

BENCHMARK_REGISTER_F(MuggleMutexFixture, LockUnLock)->Threads(1);
BENCHMARK_REGISTER_F(MuggleMutexFixture, LockUnLock)->Threads(2);
BENCHMARK_REGISTER_F(MuggleMutexFixture, LockUnLock)->Threads(4);
BENCHMARK_REGISTER_F(MuggleMutexFixture, LockUnLock)->Threads(8);

BENCHMARK_MAIN();
