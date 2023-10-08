#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <array>
#include "benchmark/benchmark.h"
#define MUGGLE_HOLD_LOG_MACRO 1
#include "muggle/c/muggle_c.h"
#include "log_msg.h"

#define ITER_COUNT 10000

std::once_flag init_flag;

#define LOG_FUNC(num)                                               \
	void log_func##num(LogMsg &msg)                                 \
	{                                                               \
		LOG_INFO("u64: %llu, i64: %lld, u32: %lu, i32: %ld, s: %s", \
				 (unsigned long long)msg.u64, (long long)msg.i64,   \
				 (unsigned long)msg.u32, (long)msg.i32, msg.s);     \
	}

EXPAND_FUNCS

class MuggleclogSyncFixture : public benchmark::Fixture {
public:
	MuggleclogSyncFixture()
	{
		GenLogMsgArray(10000, log_msgs);
	}

	void SetUp(const benchmark::State &)
	{
		std::call_once(init_flag,
					   []() { muggle_log_simple_init(-1, LOG_LEVEL_DEBUG); });
	}

public:
	std::vector<LogMsg> log_msgs;
};

BENCHMARK_DEFINE_F(MuggleclogSyncFixture, sync)(benchmark::State &state)
{
	static thread_local int idx = 0;
	const int nfuncs = sizeof(log_funcs) / sizeof(log_funcs[0]);
	for (auto _ : state) {
		idx = (idx + 1) % nfuncs;
		log_funcs[idx](log_msgs[idx]);
	}
}
BENCHMARK_REGISTER_F(MuggleclogSyncFixture, sync)
	->Threads(1)
	->Iterations(ITER_COUNT);
BENCHMARK_REGISTER_F(MuggleclogSyncFixture, sync)
	->Threads(8)
	->Iterations(ITER_COUNT);
BENCHMARK_REGISTER_F(MuggleclogSyncFixture, sync)
	->Threads(16)
	->Iterations(ITER_COUNT);

BENCHMARK_REGISTER_F(MuggleclogSyncFixture, sync)
	->Threads((std::thread::hardware_concurrency() / 2) > 0 ?
				  (std::thread::hardware_concurrency() / 2) :
				  1)
	->Iterations(ITER_COUNT);
BENCHMARK_REGISTER_F(MuggleclogSyncFixture, sync)
	->Threads(std::thread::hardware_concurrency())
	->Iterations(ITER_COUNT);
BENCHMARK_REGISTER_F(MuggleclogSyncFixture, sync)
	->Threads(std::thread::hardware_concurrency() * 2)
	->Iterations(ITER_COUNT);

BENCHMARK_MAIN();
