#include <mutex>
#include "benchmark/benchmark.h"

class StdMutexFixture : public benchmark::Fixture
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
	std::mutex mtx;
};

BENCHMARK_DEFINE_F(StdMutexFixture, LockUnLock)(benchmark::State &state)
{
	for (auto _ : state)
	{
		mtx.lock();
		ival++;
		mtx.unlock();
	}
}

BENCHMARK_REGISTER_F(StdMutexFixture, LockUnLock)->Threads(1);
BENCHMARK_REGISTER_F(StdMutexFixture, LockUnLock)->Threads(2);
BENCHMARK_REGISTER_F(StdMutexFixture, LockUnLock)->Threads(4);
BENCHMARK_REGISTER_F(StdMutexFixture, LockUnLock)->Threads(8);

BENCHMARK_MAIN();
