#include <vector>
#include <thread>
#define MUGGLE_HOLD_LOG_MACRO 1
#include "muggle/c/muggle_c.h"
#include "log_msg.h"

#define MSG_CNT 10000
#define NUM_THREAD 16

int main()
{
	muggle_log_simple_init(-1, LOG_LEVEL_DEBUG);

	int cnt = MSG_CNT;
	std::vector<LogMsg> log_msgs;
	GenLogMsgArray(cnt, log_msgs);

	std::vector<std::thread> ths;
	double elapsed_arr[NUM_THREAD];
	for (int i = 0; i < NUM_THREAD; i++) {
		ths.push_back(std::thread([&log_msgs, &elapsed_arr, cnt, i] {
			struct timespec ts1, ts2;
			timespec_get(&ts1, TIME_UTC);

			for (LogMsg &msg : log_msgs) {
				LOG_INFO("u64: %llu, i64: %lld, u32: %lu, i32: %ld, s: %s",
						 (unsigned long long)msg.u64, (long long)msg.i64,
						 (unsigned long)msg.u32, (long)msg.i32, msg.s);
			}

			timespec_get(&ts2, TIME_UTC);
			unsigned long elapsed = (ts2.tv_sec - ts1.tv_sec) * 1000000000 +
									ts2.tv_nsec - ts1.tv_nsec;
			elapsed_arr[i] = (double)elapsed / cnt;
		}));
	}

	for (std::thread &th : ths) {
		th.join();
	}

	for (int i = 0; i < NUM_THREAD; i++) {
		printf("avg elapsed: %.3f ns\n", elapsed_arr[i]);
	}

	return 0;
}
