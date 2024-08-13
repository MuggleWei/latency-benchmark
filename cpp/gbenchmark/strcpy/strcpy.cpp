#include "benchmark/benchmark.h"
#include <string.h>
#include <time.h>

#define STR_LEN 128

class StrCpyFixture : public benchmark::Fixture {
public:
	virtual void SetUp(const benchmark::State &) override
	{
		srand(time(NULL));

		for (uint32_t i = 0; i < sizeof(str_arr) / sizeof(str_arr[0]); ++i) {
			str_arr[i] = (char *)malloc(STR_LEN + 1);
			for (int j = 0; j < STR_LEN; ++j) {
				str_arr[i][j] = rand() % 26 + (int)'a';
			}
			str_arr[i][STR_LEN] = '\0';
		}

		buf[STR_LEN] = '\0';
	}

	virtual void TearDown(const benchmark::State &) override
	{
		for (uint32_t i = 0; i < sizeof(str_arr) / sizeof(str_arr[0]); ++i) {
			free(str_arr[i]);
		}
	}

public:
	uint32_t idx;
	char *str_arr[128];
	char buf[STR_LEN + 1];
};

BENCHMARK_DEFINE_F(StrCpyFixture, strncpy)(benchmark::State &state)
{
	for (auto _ : state) {
		idx = (idx + 1) % (sizeof(str_arr) / sizeof(str_arr[0]));
		strncpy(buf, str_arr[idx], sizeof(buf) - 1);
	}
}

BENCHMARK_DEFINE_F(StrCpyFixture, memcpy)(benchmark::State &state)
{
	for (auto _ : state) {
		idx = (idx + 1) % (sizeof(str_arr) / sizeof(str_arr[0]));
		memcpy(buf, str_arr[idx], sizeof(buf) - 1);
	}
}

BENCHMARK_REGISTER_F(StrCpyFixture, strncpy);
BENCHMARK_REGISTER_F(StrCpyFixture, memcpy);

BENCHMARK_MAIN();
