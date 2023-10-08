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

#define LOG_FUNC(num)                                                  \
	void log_func##num(LogMsg &msg)                                    \
	{                                                                  \
		HACLOG_INFO("u64: %llu, i64: %lld, u32: %lu, i32: %ld, s: %s", \
					(unsigned long long)msg.u64, (long long)msg.i64,   \
					(unsigned long)msg.u32, (long)msg.i32, msg.s);     \
	}

EXPAND_FUNCS

class HaclogBasicFixture : public benchmark::Fixture {
public:
	HaclogBasicFixture()
	{
		GenLogMsgArray(10000, log_msgs);
	}

	void SetUp(const benchmark::State &)
	{
		std::call_once(init_flag, []() {
			static haclog_file_handler_t file_handler = {};
			if (haclog_file_handler_init(&file_handler, "logs/haclog_basic.log",
										 "w") != 0) {
				exit(EXIT_FAILURE);
			}
			haclog_handler_set_level((haclog_handler_t *)&file_handler,
									 HACLOG_LEVEL_DEBUG);
			haclog_context_add_handler((haclog_handler_t *)&file_handler);

			haclog_backend_run();
			HACLOG_INFO("init success");
		});
	}

public:
	std::vector<LogMsg> log_msgs;
};

BENCHMARK_DEFINE_F(HaclogBasicFixture, basic)(benchmark::State &state)
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
BENCHMARK_REGISTER_F(HaclogBasicFixture, basic)->Threads(1);
BENCHMARK_REGISTER_F(HaclogBasicFixture, basic)->Threads(8);
BENCHMARK_REGISTER_F(HaclogBasicFixture, basic)->Threads(16);

BENCHMARK_REGISTER_F(HaclogBasicFixture, basic)
	->Threads((std::thread::hardware_concurrency() / 2) > 0 ?
				  (std::thread::hardware_concurrency() / 2) :
				  1);
BENCHMARK_REGISTER_F(HaclogBasicFixture, basic)
	->Threads(std::thread::hardware_concurrency());
BENCHMARK_REGISTER_F(HaclogBasicFixture, basic)
	->Threads(std::thread::hardware_concurrency() * 2);

BENCHMARK_MAIN();
