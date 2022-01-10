#include "benchmark/benchmark.h"
#include "muggle/c/muggle_c.h"

class MuggleSpinLockFixture : public benchmark::Fixture
{
public:
	void SetUp(const benchmark::State &)
	{
		ival = 0;
		muggle_spinlock_init(&spinlock);
	}

	void TearDown(const benchmark::State &)
	{}

public:
	int ival;
	muggle_atomic_int spinlock;
};

BENCHMARK_DEFINE_F(MuggleSpinLockFixture, LockUnLock)(benchmark::State &state)
{
	for (auto _ : state)
	{
		muggle_spinlock_lock(&spinlock);
		ival++;
		muggle_spinlock_unlock(&spinlock);
	}
}

BENCHMARK_REGISTER_F(MuggleSpinLockFixture, LockUnLock)->Threads(1);
BENCHMARK_REGISTER_F(MuggleSpinLockFixture, LockUnLock)->Threads(2);
BENCHMARK_REGISTER_F(MuggleSpinLockFixture, LockUnLock)->Threads(4);
BENCHMARK_REGISTER_F(MuggleSpinLockFixture, LockUnLock)->Threads(8);

BENCHMARK_MAIN();
