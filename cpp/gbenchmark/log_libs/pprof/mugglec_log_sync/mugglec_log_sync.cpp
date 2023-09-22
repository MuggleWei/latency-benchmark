#include <vector>
#define MUGGLE_HOLD_LOG_MACRO 1
#include "muggle/c/muggle_c.h"
#include "log_msg.h"

int main()
{
	muggle_log_simple_init(-1, LOG_LEVEL_DEBUG);

	std::vector<LogMsg> log_msgs;
	GenLogMsgArray(100000, log_msgs);

	for (LogMsg &msg : log_msgs) {
		LOG_INFO("u64: %llu, i64: %lld, u32: %lu, i32: %ld, s: %s",
				 (unsigned long long)msg.u64, (long long)msg.i64,
				 (unsigned long)msg.u32, (long)msg.i32, msg.s);
	}

	return 0;
}
