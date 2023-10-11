#include "benchmark/benchmark.h"
#include <array>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#define MUGGLE_HOLD_LOG_MACRO 1
#include "log_msg.h"
#include "muggle/c/muggle_c.h"

#define ITER_COUNT 10000
#define REPEAT_COUNT 5

#define MIN_TIME 3.0

std::once_flag init_flag;

muggle_logger_t *s_logger = nullptr;

#define LOG_FUNC(num)                                                 \
	void log_func##num(LogMsg &msg)                                   \
	{                                                                 \
		MUGGLE_LOG(s_logger, LOG_LEVEL_DEBUG,                         \
				   "u64: %llu, i64: %lld, u32: %lu, i32: %ld, s: %s", \
				   (unsigned long long)msg.u64, (long long)msg.i64,   \
				   (unsigned long)msg.u32, (long)msg.i32, msg.s);     \
	}

EXPAND_FUNCS

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

			s_logger = my_async_logger();
		});
	}

public:
	std::vector<LogMsg> log_msgs;
};

BENCHMARK_DEFINE_F(MuggleclogAsyncFixture, async)(benchmark::State &state)
{
	static thread_local int idx = 0;
	const int nfuncs = sizeof(log_funcs) / sizeof(log_funcs[0]);
	for (auto _ : state) {
		idx = (idx + 1) % nfuncs;
		log_funcs[idx](log_msgs[idx]);
	}
}

// min time
BENCHMARK_REGISTER_F(MuggleclogAsyncFixture, async)->Threads(1)->MinTime(MIN_TIME);
BENCHMARK_REGISTER_F(MuggleclogAsyncFixture, async)->Threads(2)->MinTime(MIN_TIME);
BENCHMARK_REGISTER_F(MuggleclogAsyncFixture, async)
	->Threads((std::thread::hardware_concurrency() / 2) > 0 ?
				  (std::thread::hardware_concurrency() / 2) :
				  1)
	->MinTime(MIN_TIME);
BENCHMARK_REGISTER_F(MuggleclogAsyncFixture, async)
	->Threads(std::thread::hardware_concurrency() - 1 > 0 ?
				  (std::thread::hardware_concurrency() - 1) :
				  1)
	->MinTime(MIN_TIME);

// iteration * repeat
BENCHMARK_REGISTER_F(MuggleclogAsyncFixture, async)
	->Threads(1)
	->Iterations(ITER_COUNT)
	->Repetitions(REPEAT_COUNT);
BENCHMARK_REGISTER_F(MuggleclogAsyncFixture, async)
	->Threads(8)
	->Iterations(ITER_COUNT)
	->Repetitions(REPEAT_COUNT);
BENCHMARK_REGISTER_F(MuggleclogAsyncFixture, async)
	->Threads(16)
	->Iterations(ITER_COUNT)
	->Repetitions(REPEAT_COUNT);

BENCHMARK_REGISTER_F(MuggleclogAsyncFixture, async)
	->Threads((std::thread::hardware_concurrency() / 2) > 0 ?
				  (std::thread::hardware_concurrency() / 2) :
				  1)
	->Iterations(ITER_COUNT)
	->Repetitions(REPEAT_COUNT);
BENCHMARK_REGISTER_F(MuggleclogAsyncFixture, async)
	->Threads(std::thread::hardware_concurrency())
	->Iterations(ITER_COUNT)
	->Repetitions(REPEAT_COUNT);
BENCHMARK_REGISTER_F(MuggleclogAsyncFixture, async)
	->Threads(std::thread::hardware_concurrency() * 2)
	->Iterations(ITER_COUNT)
	->Repetitions(REPEAT_COUNT);

BENCHMARK_MAIN();
