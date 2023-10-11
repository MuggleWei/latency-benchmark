#include <atomic>
#include <thread>
#include <vector>
#include "benchmark/benchmark.h"
#include "log_msg.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

#define ITER_COUNT 10000
#define REPEAT_COUNT 5

#define MIN_TIME 3.0

std::once_flag init_flag;

#define LOG_FUNC(num)                                                \
	void log_func##num(LogMsg &msg)                                  \
	{                                                                \
		SPDLOG_INFO("u64: {}, i64: {}, u32: {}, i32: {}, s: {}",     \
					(unsigned long long)msg.u64, (long long)msg.i64, \
					(unsigned long)msg.u32, (long)msg.i32, msg.s);   \
	}

EXPAND_FUNCS

class SpdlogSyncFixture : public benchmark::Fixture {
public:
	SpdlogSyncFixture()
	{
		GenLogMsgArray(10000, log_msgs);
	}

	void SetUp(const benchmark::State &)
	{
		std::call_once(init_flag, []() {
			try {
				auto logger =
					spdlog::basic_logger_mt("root", "logs/spdlog_basic.log");
				logger->set_level(spdlog::level::debug);
				logger->set_pattern("%l|%E.%F|%s|%!|%t - %v");

				spdlog::set_default_logger(logger);
			} catch (const spdlog::spdlog_ex &ex) {
				fprintf(stderr, "log init failed: %s\n", ex.what());
			}
			SPDLOG_INFO("init success");
		});
	}

public:
	std::vector<LogMsg> log_msgs;
};

BENCHMARK_DEFINE_F(SpdlogSyncFixture, sync)(benchmark::State &state)
{
	static thread_local int idx = 0;
	const int nfuncs = sizeof(log_funcs) / sizeof(log_funcs[0]);
	for (auto _ : state) {
		idx = (idx + 1) % nfuncs;
		log_funcs[idx](log_msgs[idx]);
	}
}

// min time
BENCHMARK_REGISTER_F(SpdlogSyncFixture, sync)->Threads(1)->MinTime(MIN_TIME);
BENCHMARK_REGISTER_F(SpdlogSyncFixture, sync)->Threads(2)->MinTime(MIN_TIME);
BENCHMARK_REGISTER_F(SpdlogSyncFixture, sync)
	->Threads((std::thread::hardware_concurrency() / 2) > 0 ?
				  (std::thread::hardware_concurrency() / 2) :
				  1)
	->MinTime(MIN_TIME);
BENCHMARK_REGISTER_F(SpdlogSyncFixture, sync)
	->Threads(std::thread::hardware_concurrency() - 1 > 0 ?
				  (std::thread::hardware_concurrency() - 1) :
				  1)
	->MinTime(MIN_TIME);

// iteration * repeat
BENCHMARK_REGISTER_F(SpdlogSyncFixture, sync)
	->Threads(1)
	->Iterations(ITER_COUNT)
	->Repetitions(REPEAT_COUNT);
BENCHMARK_REGISTER_F(SpdlogSyncFixture, sync)
	->Threads(8)
	->Iterations(ITER_COUNT)
	->Repetitions(REPEAT_COUNT);
BENCHMARK_REGISTER_F(SpdlogSyncFixture, sync)
	->Threads(16)
	->Iterations(ITER_COUNT)
	->Repetitions(REPEAT_COUNT);

BENCHMARK_REGISTER_F(SpdlogSyncFixture, sync)
	->Threads((std::thread::hardware_concurrency() / 2) > 0 ?
				  (std::thread::hardware_concurrency() / 2) :
				  1)
	->Iterations(ITER_COUNT)
	->Repetitions(REPEAT_COUNT);
BENCHMARK_REGISTER_F(SpdlogSyncFixture, sync)
	->Threads(std::thread::hardware_concurrency())
	->Iterations(ITER_COUNT)
	->Repetitions(REPEAT_COUNT);
BENCHMARK_REGISTER_F(SpdlogSyncFixture, sync)
	->Threads(std::thread::hardware_concurrency() * 2)
	->Iterations(ITER_COUNT)
	->Repetitions(REPEAT_COUNT);

BENCHMARK_MAIN();