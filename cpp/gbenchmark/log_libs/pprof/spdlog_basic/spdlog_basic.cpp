#include <vector>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "log_msg.h"

int main()
{
	auto logger = spdlog::basic_logger_mt("root", "logs/spdlog_basic.log");
	logger->set_level(spdlog::level::debug);
	logger->set_pattern("%l|%E.%F|%s|%!|%t - %v");
	spdlog::set_default_logger(logger);

	std::vector<LogMsg> log_msgs;
	GenLogMsgArray(100000, log_msgs);

	for (LogMsg &msg : log_msgs) {
		SPDLOG_INFO("u64: {}, i64: {}, u32: {}, i32: {}, s: {}",
					(unsigned long long)msg.u64, (long long)msg.i64,
					(unsigned long)msg.u32, (long)msg.i32, msg.s);
	}

	return 0;
}
