#include "log_msg.h"
#include <time.h>
#include <stdlib.h>

void GenLogMsgArray(uint32_t cnt, std::vector<LogMsg> &log_msgs)
{
	log_msgs.clear();

	srand(time(NULL));
	for (uint32_t i = 0; i < cnt; i++) {
		LogMsg msg = {};
		msg.u64 = (uint64_t)i;
		msg.u32 = (uint32_t)i;
		msg.i64 = (int64_t)i;
		msg.i32 = (int32_t)i;
		for (int i = 0; i < (int)sizeof(msg.s) - 1; i++) {
			msg.s[i] = (rand() % 26) + 'a';
		}
		log_msgs.push_back(msg);
	}
}
