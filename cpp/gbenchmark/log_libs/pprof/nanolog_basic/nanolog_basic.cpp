#include <array>
#include <vector>
#include "log_msg.h"
#include "NanoLogCpp17.h"
using namespace NanoLog::LogLevels;

int main()
{
	NanoLog::setLogFile("logs/nanolog_basic.log");
	NanoLog::preallocate();
	NanoLog::setLogLevel(NOTICE);
	NANO_LOG(NOTICE, "init success");

	std::vector<LogMsg> log_msgs;
	GenLogMsgArray(100000, log_msgs);

	for (LogMsg &msg : log_msgs) {
		NANO_LOG(NOTICE, "u64: %llu, i64: %lld, u32: %lu, i32: %ld, s: %s",
				 (unsigned long long)msg.u64, (long long)msg.i64,
				 (unsigned long)msg.u32, (long)msg.i32, msg.s);
	}

	return 0;
}
