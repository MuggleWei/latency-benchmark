#include <time.h>
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include "muggle/muggle_cc.h"

int64_t g_tmp = 0;
volatile int64_t g_v_tmp = 0;
std::mutex g_mtx;

void run_unlock(int64_t cnt)
{
	struct timespec start_ts, end_ts;
	int64_t elapsed = 0;

	timespec_get(&start_ts, TIME_UTC);
	for (int64_t i = 0; i < cnt; ++i)
	{
		++g_tmp;
	}
	timespec_get(&end_ts, TIME_UTC);
	elapsed = (int64_t)(end_ts.tv_sec - start_ts.tv_sec) * 1000000000 + int64_t(end_ts.tv_nsec - start_ts.tv_nsec);
	std::cout << "loop without lock use ns: " << elapsed << ", g_tmp=" << g_tmp << std::endl;;
}

void run_volatile(int64_t cnt)
{
	struct timespec start_ts, end_ts;
	int64_t elapsed = 0;

	timespec_get(&start_ts, TIME_UTC);
	for (int64_t i = 0; i < cnt; ++i)
	{
		++g_v_tmp;
	}
	timespec_get(&end_ts, TIME_UTC);
	elapsed = (int64_t)(end_ts.tv_sec - start_ts.tv_sec) * 1000000000 + int64_t(end_ts.tv_nsec - start_ts.tv_nsec);
	std::cout << "loop volatile without lock use ns: " << elapsed << ", g_v_tmp=" << g_v_tmp << std::endl;;
}

void run_lock(int64_t cnt)
{
	struct timespec start_ts, end_ts;
	int64_t elapsed = 0;

	timespec_get(&start_ts, TIME_UTC);
	for (int64_t i = 0; i < cnt; ++i)
	{
		std::unique_lock<std::mutex> lock(g_mtx);
		++g_v_tmp;
		lock.unlock();
	}
	timespec_get(&end_ts, TIME_UTC);
	elapsed = (int64_t)(end_ts.tv_sec - start_ts.tv_sec) * 1000000000 + int64_t(end_ts.tv_nsec - start_ts.tv_nsec);
	std::cout << "loop with lock use ns: " << elapsed << ", g_v_tmp=" << g_v_tmp << std::endl;
}

void run_cas(int64_t cnt)
{
	struct timespec start_ts, end_ts;
	int64_t elapsed = 0;
	int64_t tmp;

	timespec_get(&start_ts, TIME_UTC);
	for (int64_t i = 0; i < cnt; ++i)
	{
		do {
			tmp = g_v_tmp;
		} while (MUGGLE_ATOMIC_CAS_64(g_v_tmp, tmp, tmp+1) != tmp);
	}
	timespec_get(&end_ts, TIME_UTC);
	elapsed = (int64_t)(end_ts.tv_sec - start_ts.tv_sec) * 1000000000 + int64_t(end_ts.tv_nsec - start_ts.tv_nsec);
	std::cout << "loop with CAS use ns: " << elapsed << ", g_v_tmp=" << g_v_tmp << std::endl;
}

int main(int argc, char *argv[])
{
	int64_t cnt = 1000000 * 2;

	if (argc != 3)
	{
		std::cout << "usage: " << argv[0] << " [u(unlock) | v(volatile) | l(lock) | c(CAS)]   [s(single thread) | m(two thread)]" << std::endl;
		return 1;
	}

	switch (argv[1][0])
	{
	case 'u':
		{
			if (argv[2][0] == 's')
			{
				std::cout << "single thread with unlock" << std::endl;
				std::thread t(run_unlock, cnt);
				t.join();
			}
		}break;
	case 'v':
		{
			if (argv[2][0] == 's')
			{
				std::cout << "single thread with volatile unlock" << std::endl;
				std::thread t(run_volatile, cnt);
				t.join();
			}
		}break;
	case 'l':
		{
			if (argv[2][0] == 's')
			{
				std::cout << "single thread with lock" << std::endl;
				std::thread t(run_lock, cnt);
				t.join();
			}
			else
			{
				std::cout << "two threads with lock" << std::endl;
				std::thread t1(run_lock, cnt/2);
				std::thread t2(run_lock, cnt/2);
				t1.join();
				t2.join();
			}
		}break;
	case 'c':
		{
			if (argv[2][0] == 's')
			{
				std::cout << "single thread with CAS" << std::endl;
				std::thread t(run_cas, cnt);
				t.join();
			}
			else
			{
				std::cout << "two threads with CAS" << std::endl;
				std::thread t1(run_cas, cnt/2);
				std::thread t2(run_cas, cnt/2);
				t1.join();
				t2.join();
			}
		}break;
	}

	return 0;
}
