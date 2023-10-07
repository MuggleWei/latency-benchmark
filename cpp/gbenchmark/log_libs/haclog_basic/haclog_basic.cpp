#include "haclog/haclog_log.h"
#include "haclog/haclog_thread_context.h"
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include "benchmark/benchmark.h"
#include "log_msg.h"
#include "haclog/haclog.h"

std::once_flag init_flag;

#define LOG_FUNC(num)                                                \
	void log_func##num(LogMsg &msg)                                  \
	{                                                                \
		HACLOG_INFO("u64: {}, i64: {}, u32: {}, i32: {}, s: {}",     \
					(unsigned long long)msg.u64, (long long)msg.i64, \
					(unsigned long)msg.u32, (long)msg.i32, msg.s);   \
	}

EXPAND_FUNCS

class SpdlogBasicFixture : public benchmark::Fixture {
public:
	SpdlogBasicFixture()
	{
		GenLogMsgArray(10000, log_msgs);
	}

	void SetUp(const benchmark::State &)
	{
		std::call_once(init_flag, []() {
			haclog_backend_run();
			HACLOG_INFO("init success");
		});
	}

public:
	std::vector<LogMsg> log_msgs;
};

BENCHMARK_DEFINE_F(SpdlogBasicFixture, basic)(benchmark::State &state)
{
	haclog_thread_context_init();

	const int nfuncs = sizeof(log_funcs) / sizeof(log_funcs[0]);
	for (auto _ : state) {
		for (LogMsg &msg : log_msgs) {
			log_funcs[msg.i32 % nfuncs](msg);
		}
	}

	haclog_thread_context_cleanup();
}
BENCHMARK_REGISTER_F(SpdlogBasicFixture, basic)->Threads(1);
BENCHMARK_REGISTER_F(SpdlogBasicFixture, basic)->Threads(8);
BENCHMARK_REGISTER_F(SpdlogBasicFixture, basic)->Threads(16);

BENCHMARK_REGISTER_F(SpdlogBasicFixture, basic)
	->Threads((std::thread::hardware_concurrency() / 2) > 0 ?
				  (std::thread::hardware_concurrency() / 2) :
				  1);
BENCHMARK_REGISTER_F(SpdlogBasicFixture, basic)
	->Threads(std::thread::hardware_concurrency());
BENCHMARK_REGISTER_F(SpdlogBasicFixture, basic)
	->Threads(std::thread::hardware_concurrency() * 2);

BENCHMARK_MAIN();
