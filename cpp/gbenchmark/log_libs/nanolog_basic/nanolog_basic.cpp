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

#define LOG_FUNC(num)                                                       \
	void log_func##num(LogMsg &msg)                                         \
	{                                                                       \
		NANO_LOG(NOTICE, "u64: %llu, i64: %lld, u32: %lu, i32: %ld, s: %s", \
				 (unsigned long long)msg.u64, (long long)msg.i64,           \
				 (unsigned long)msg.u32, (long)msg.i32, msg.s);             \
	}

EXPAND_FUNCS

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
	const int nfuncs = sizeof(log_funcs) / sizeof(log_funcs[0]);
	for (auto _ : state) {
		for (LogMsg &msg : log_msgs) {
			log_funcs[msg.i32 % nfuncs](msg);
		}
	}
}
BENCHMARK_REGISTER_F(NanologBasicFixture, sync)->Threads(1);
BENCHMARK_REGISTER_F(NanologBasicFixture, sync)->Threads(8);
BENCHMARK_REGISTER_F(NanologBasicFixture, sync)->Threads(16);

BENCHMARK_REGISTER_F(NanologBasicFixture, sync)
	->Threads((std::thread::hardware_concurrency() / 2) > 0 ?
				  (std::thread::hardware_concurrency() / 2) :
				  1);
BENCHMARK_REGISTER_F(NanologBasicFixture, sync)
	->Threads(std::thread::hardware_concurrency());
BENCHMARK_REGISTER_F(NanologBasicFixture, sync)
	->Threads(std::thread::hardware_concurrency() * 2);

BENCHMARK_MAIN();
