#include "benchmark/benchmark.h"
#include "muggle/c/muggle_c.h"

#if MUGGLE_C_HAVE_SYNC_OBJ

class MuggleSyncFixture : public benchmark::Fixture {
public:
	void SetUp(const benchmark::State &)
	{
		muggle_synclock_init(&futex);
		ival = 0;
	}

	void TearDown(const benchmark::State &)
	{
	}

public:
	int ival;
	muggle_sync_t futex;
};

BENCHMARK_DEFINE_F(MuggleSyncFixture, LockUnLock)(benchmark::State &state)
{
	for (auto _ : state) {
		muggle_synclock_lock(&futex);
		ival++;
		muggle_synclock_unlock(&futex);
	}
}
BENCHMARK_REGISTER_F(MuggleSyncFixture, LockUnLock)->Threads(1);
BENCHMARK_REGISTER_F(MuggleSyncFixture, LockUnLock)->Threads(2);
BENCHMARK_REGISTER_F(MuggleSyncFixture, LockUnLock)->Threads(4);
BENCHMARK_REGISTER_F(MuggleSyncFixture, LockUnLock)->Threads(8);

#endif

BENCHMARK_MAIN();
