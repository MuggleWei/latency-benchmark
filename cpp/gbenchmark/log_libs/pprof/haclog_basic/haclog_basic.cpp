#include <stdlib.h>
#include <array>
#include <vector>
#include "log_msg.h"
#include "haclog/haclog.h"

int main()
{
	static haclog_file_handler_t file_handler = {};
	if (haclog_file_handler_init(&file_handler, "logs/haclog_basic.log", "w") !=
		0) {
		exit(EXIT_FAILURE);
	}
	haclog_handler_set_level((haclog_handler_t *)&file_handler,
							 HACLOG_LEVEL_DEBUG);
	haclog_context_add_handler((haclog_handler_t *)&file_handler);
	haclog_backend_run();

	std::vector<LogMsg> log_msgs;
	GenLogMsgArray(100000, log_msgs);

	haclog_thread_context_init();

	for (LogMsg &msg : log_msgs) {
		HACLOG_INFO("u64: %llu, i64: %lld, u32: %lu, i32: %ld, s: %s",
					(unsigned long long)msg.u64, (long long)msg.i64,
					(unsigned long)msg.u32, (long)msg.i32, msg.s);
	}

	haclog_thread_context_cleanup();

	return 0;
}
