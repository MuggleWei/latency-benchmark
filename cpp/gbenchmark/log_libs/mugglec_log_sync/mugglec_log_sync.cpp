#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <array>
#include "benchmark/benchmark.h"
#define MUGGLE_HOLD_LOG_MACRO 1
#include "muggle/c/muggle_c.h"
#include "log_msg.h"

std::once_flag init_flag;

class MuggleclogSyncFixture : public benchmark::Fixture {
public:
	MuggleclogSyncFixture()
	{
		GenLogMsgArray(10000, log_msgs);
	}

	void SetUp(const benchmark::State &)
	{
		std::call_once(init_flag, []() {
			muggle_log_simple_init(-1, LOG_LEVEL_DEBUG);
		});
	}

public:
	std::vector<LogMsg> log_msgs;
};

BENCHMARK_DEFINE_F(MuggleclogSyncFixture, sync)(benchmark::State &state)
{
	for (auto _ : state) {
		for (LogMsg &msg : log_msgs) {
			LOG_INFO("u64: %llu, i64: %lld, u32: %lu, i32: %ld, s: %s",
					 (unsigned long long)msg.u64, (long long)msg.i64,
					 (unsigned long)msg.u32, (long)msg.i32, msg.s);
		}
	}
}
BENCHMARK_REGISTER_F(MuggleclogSyncFixture, sync)->Threads(1);
BENCHMARK_REGISTER_F(MuggleclogSyncFixture, sync)->Threads(8);
BENCHMARK_REGISTER_F(MuggleclogSyncFixture, sync)->Threads(16);
BENCHMARK_REGISTER_F(MuggleclogSyncFixture, sync)->Threads(32);

BENCHMARK_MAIN();
