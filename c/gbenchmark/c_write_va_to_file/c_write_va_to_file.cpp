#include "benchmark/benchmark.h"
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <vector>

#define NUM_MSG 100000

struct LogMsg {
	uint64_t u64;
	uint32_t u32;
	int64_t i64;
	int32_t i32;
	char s[128];
};

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

class CWriteVaToFileFixture : public benchmark::Fixture {
public:
	void SetUp(const benchmark::State &)
	{
		GenLogMsgArray(NUM_MSG, msgs);

		fp = fopen("c_write_va_to_file.log", "w");

		idx = 0;
	}

	void TearDown(const benchmark::State &)
	{
		fclose(fp);

		msgs.clear();
	}

public:
	std::vector<LogMsg> msgs;
	FILE *fp;
	unsigned long long idx;
};

static void write_fvprintf(FILE *fp, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(fp, fmt, args);
	va_end(args);
}

BENCHMARK_DEFINE_F(CWriteVaToFileFixture, fvprintf)(benchmark::State &state)
{
	for (auto _ : state) {
		LogMsg &msg = msgs[idx++];
		write_fvprintf(fp, "u64: %llu, i64: %lld, u32: %lu, i32: %ld, s: %s\n",
					   (unsigned long long)msg.u64, (long long)msg.i64,
					   (unsigned long)msg.u32, (long)msg.i32, msg.s);

		idx = (idx + 1) % msgs.size();
	}
}
BENCHMARK_DEFINE_F(CWriteVaToFileFixture, snprintf_write)
(benchmark::State &state)
{
	char buf[2048];
	for (auto _ : state) {
		LogMsg &msg = msgs[idx++];
		int n = snprintf(buf, sizeof(buf),
						 "u64: %llu, i64: %lld, u32: %lu, i32: %ld, s: %s\n",
						 (unsigned long long)msg.u64, (long long)msg.i64,
						 (unsigned long)msg.u32, (long)msg.i32, msg.s);
		fwrite(buf, 1, n, fp);

		idx = (idx + 1) % msgs.size();
	}
}

BENCHMARK_REGISTER_F(CWriteVaToFileFixture, fvprintf)
	->Threads(1)
	->Iterations(10 * NUM_MSG);

BENCHMARK_REGISTER_F(CWriteVaToFileFixture, snprintf_write)
	->Threads(1)
	->Iterations(10 * NUM_MSG);

BENCHMARK_MAIN();
