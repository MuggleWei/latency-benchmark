#include "benchmark/benchmark.h"
#include <array>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#define MUGGLE_HOLD_LOG_MACRO 1
#include "log_msg.h"
#include "muggle/c/muggle_c.h"

std::once_flag init_flag;

muggle_logger_t *my_async_logger()
{
	static muggle_async_logger_t async_logger;
	return (muggle_logger_t *)&async_logger;
}

class MuggleclogAsyncFixture : public benchmark::Fixture {
public:
	MuggleclogAsyncFixture()
	{
		GenLogMsgArray(10000, log_msgs);
	}

	void SetUp(const benchmark::State &)
	{
		std::call_once(init_flag, []() {
			static muggle_log_file_handler_t file_handler;
			muggle_log_file_handler_init(&file_handler, "logs/muggle_async.log",
										 "w");
			muggle_log_handler_set_level((muggle_log_handler_t *)&file_handler,
										 LOG_LEVEL_DEBUG);

			muggle_logger_t *logger = my_async_logger();
			muggle_async_logger_init((muggle_async_logger_t *)logger, 4096);
			logger->add_handler(logger, (muggle_log_handler_t *)&file_handler);
		});
	}

public:
	std::vector<LogMsg> log_msgs;
};

BENCHMARK_DEFINE_F(MuggleclogAsyncFixture, sync)(benchmark::State &state)
{
	muggle_logger_t *logger = my_async_logger();
	for (auto _ : state) {
		for (LogMsg &msg : log_msgs) {
			MUGGLE_LOG(logger, LOG_LEVEL_DEBUG,
					   "u64: %llu, i64: %lld, u32: %lu, i32: %ld, s: %s",
					   (unsigned long long)msg.u64, (long long)msg.i64,
					   (unsigned long)msg.u32, (long)msg.i32, msg.s);
		}
	}
}
BENCHMARK_REGISTER_F(MuggleclogAsyncFixture, sync)->Threads(1);
BENCHMARK_REGISTER_F(MuggleclogAsyncFixture, sync)->Threads(8);
BENCHMARK_REGISTER_F(MuggleclogAsyncFixture, sync)->Threads(16);
BENCHMARK_REGISTER_F(MuggleclogAsyncFixture, sync)->Threads(32);

BENCHMARK_MAIN();
