#include "haclog/haclog_thread_context.h"
#include <stdlib.h>
#include <array>
#include <vector>
#include <thread>
#include "log_msg.h"
#include "haclog/haclog.h"

#define TOTAL_PER_ROUND 10000
static int MSG_CNT = 0;
static int NUM_THREAD = 0;

// #define TOTAL_PER_ROUND 0
// static int MSG_CNT = 40000;
// static int NUM_THREAD = 2;

#define ROUND 16
#define ROUND_INTERVAL (50 * 1000 * 1000)

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

#if TOTAL_PER_ROUND
	NUM_THREAD = std::thread::hardware_concurrency();
	MSG_CNT = TOTAL_PER_ROUND / NUM_THREAD;
#endif

	int cnt = MSG_CNT;
	std::vector<LogMsg> log_msgs;
	GenLogMsgArray(cnt, log_msgs);

	std::vector<std::thread> ths;
	double elapsed_arr[NUM_THREAD * ROUND];
	for (int i = 0; i < NUM_THREAD; i++) {
		ths.push_back(std::thread([&log_msgs, &elapsed_arr, cnt, i] {
			haclog_thread_context_init();

			for (int r = 0; r < ROUND; r++) {
				struct timespec ts1, ts2;
				timespec_get(&ts1, TIME_UTC);

				for (LogMsg &msg : log_msgs) {
					HACLOG_INFO(
						"u64: %llu, i64: %lld, u32: %lu, i32: %ld, s: %s",
						(unsigned long long)msg.u64, (long long)msg.i64,
						(unsigned long)msg.u32, (long)msg.i32, msg.s);
				}

				timespec_get(&ts2, TIME_UTC);
				unsigned long elapsed = (ts2.tv_sec - ts1.tv_sec) * 1000000000 +
										ts2.tv_nsec - ts1.tv_nsec;

				elapsed_arr[i * ROUND + r] = (double)elapsed / cnt;
				std::this_thread::sleep_for(
					std::chrono::nanoseconds(ROUND_INTERVAL));
			}

			haclog_thread_context_cleanup();
		}));
	}

	for (std::thread &th : ths) {
		th.join();
	}

	for (int i = 0; i < NUM_THREAD; i++) {
		printf("[%d] elapsed:", i);
		for (int r = 0; r < ROUND; ++r) {
			printf(" %8.3f |", elapsed_arr[i * ROUND + r]);
		}
		printf("\n");
	}

	return 0;
}
