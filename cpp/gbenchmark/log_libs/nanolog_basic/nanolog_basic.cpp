#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <array>
#include "benchmark/benchmark.h"
#include "log_msg.h"
#include "NanoLogCpp17.h"
using namespace NanoLog::LogLevels;

std::once_flag init_flag;

class NanologBasicFixture : public benchmark::Fixture {
public:
	NanologBasicFixture()
	{
		GenLogMsgArray(10000, log_msgs);
	}

	void SetUp(const benchmark::State &)
	{
		std::call_once(init_flag, []() {
			NanoLog::setLogFile("logs/nanolog_basic.log");
			NanoLog::preallocate();
			NanoLog::setLogLevel(NOTICE);
			NANO_LOG(NOTICE, "init success");
		});
	}

public:
	std::vector<LogMsg> log_msgs;
};

BENCHMARK_DEFINE_F(NanologBasicFixture, sync)(benchmark::State &state)
{
	for (auto _ : state) {
		for (LogMsg &msg : log_msgs) {
			NANO_LOG(NOTICE, "u64: %llu, i64: %lld, u32: %lu, i32: %ld, s: %s",
					 (unsigned long long)msg.u64, (long long)msg.i64,
					 (unsigned long)msg.u32, (long)msg.i32, msg.s);
		}
	}
}
BENCHMARK_REGISTER_F(NanologBasicFixture, sync)->Threads(1);
BENCHMARK_REGISTER_F(NanologBasicFixture, sync)->Threads(8);
BENCHMARK_REGISTER_F(NanologBasicFixture, sync)->Threads(16);
BENCHMARK_REGISTER_F(NanologBasicFixture, sync)->Threads(32);

BENCHMARK_MAIN();
