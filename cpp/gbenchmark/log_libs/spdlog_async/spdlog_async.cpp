#include <atomic>
#include <thread>
#include <vector>
#include "benchmark/benchmark.h"
#include "log_msg.h"
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"

std::once_flag init_flag;

class SpdlogAsyncFixture : public benchmark::Fixture {
public:
	SpdlogAsyncFixture()
	{
		GenLogMsgArray(10000, log_msgs);
	}

	void SetUp(const benchmark::State &)
	{
		std::call_once(init_flag, []() {
			try {
				auto logger = spdlog::basic_logger_mt<spdlog::async_factory>(
					"root", "logs/spdlog_async.log");
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

BENCHMARK_DEFINE_F(SpdlogAsyncFixture, async)(benchmark::State &state)
{
	for (auto _ : state) {
		for (LogMsg &msg : log_msgs) {
			SPDLOG_INFO("u64: {}, i64: {}, u32: {}, i32: {}, s: {}",
						(unsigned long long)msg.u64, (long long)msg.i64,
						(unsigned long)msg.u32, (long)msg.i32, msg.s);
		}
	}
}
BENCHMARK_REGISTER_F(SpdlogAsyncFixture, async)->Threads(1);
BENCHMARK_REGISTER_F(SpdlogAsyncFixture, async)->Threads(8);
BENCHMARK_REGISTER_F(SpdlogAsyncFixture, async)->Threads(16);

BENCHMARK_REGISTER_F(SpdlogAsyncFixture, async)
	->Threads((std::thread::hardware_concurrency() / 2) > 0 ?
				  (std::thread::hardware_concurrency() / 2) :
				  1);
BENCHMARK_REGISTER_F(SpdlogAsyncFixture, async)
	->Threads(std::thread::hardware_concurrency());
BENCHMARK_REGISTER_F(SpdlogAsyncFixture, async)
	->Threads(std::thread::hardware_concurrency() * 2);

BENCHMARK_MAIN();
