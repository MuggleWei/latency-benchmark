#ifndef BENCHMARK_LOG_MSG_H_
#define BENCHMARK_LOG_MSG_H_

#include <stdint.h>
#include <vector>

struct LogMsg {
	uint64_t u64;
	uint32_t u32;
	int64_t i64;
	int32_t i32;
	char s[128];
};

void GenLogMsgArray(uint32_t cnt, std::vector<LogMsg> &log_msgs);

#endif // !BENCHMARK_LOG_MSG_H_
