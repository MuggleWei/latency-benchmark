#include <chrono>
#include <stdint.h>
#include "benchmark/benchmark.h"

#define MEASURE_CNT 1000

class TimeElapsedMeasureFixture : public benchmark::Fixture {
public:
	virtual void SetUp(const benchmark::State &) override
	{
		double_arr = (double *)malloc(sizeof(double) * MEASURE_CNT);
		int64_arr = (int64_t *)malloc(sizeof(int64_t) * MEASURE_CNT);
		idx = 0;

		steady_start = std::chrono::steady_clock::now();
		high_resolution_start = std::chrono::high_resolution_clock::now();
		timespec_get(&ts_start, TIME_UTC);
	}

	virtual void TearDown(const benchmark::State &) override
	{
		free(double_arr);
		free(int64_arr);
	}

public:
	double *double_arr;
	int64_t *int64_arr;
	uint32_t idx;

	std::chrono::steady_clock::time_point steady_start;
	std::chrono::high_resolution_clock::time_point high_resolution_start;
	struct timespec ts_start;
};

BENCHMARK_DEFINE_F(TimeElapsedMeasureFixture, SteadyDouble)
(benchmark::State &state)
{
	for (auto _ : state) {
		auto cur = std::chrono::steady_clock::now();
		std::chrono::duration<double> diff = cur - steady_start;
		double_arr[idx++] = diff.count();
	}
}

BENCHMARK_DEFINE_F(TimeElapsedMeasureFixture, SteadyInt)
(benchmark::State &state)
{
	for (auto _ : state) {
		auto cur = std::chrono::steady_clock::now();
		std::chrono::duration<int64_t, std::nano> diff = cur - steady_start;
		int64_arr[idx++] = diff.count();
	}
}

BENCHMARK_DEFINE_F(TimeElapsedMeasureFixture, HighResolutionDouble)
(benchmark::State &state)
{
	for (auto _ : state) {
		auto cur = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> diff = cur - high_resolution_start;
		double_arr[idx++] = diff.count();
	}
}

BENCHMARK_DEFINE_F(TimeElapsedMeasureFixture, HighResolutionInt)
(benchmark::State &state)
{
	for (auto _ : state) {
		auto cur = std::chrono::high_resolution_clock::now();
		std::chrono::duration<int64_t, std::nano> diff =
			cur - high_resolution_start;
		int64_arr[idx++] = diff.count();
	}
}

BENCHMARK_DEFINE_F(TimeElapsedMeasureFixture, timespec_int)
(benchmark::State &state)
{
	for (auto _ : state) {
		struct timespec cur;
		timespec_get(&cur, TIME_UTC);
		int64_arr[idx++] = (cur.tv_sec - ts_start.tv_sec) * 1000000000 +
						   cur.tv_nsec - ts_start.tv_nsec;
	}
}

BENCHMARK_REGISTER_F(TimeElapsedMeasureFixture, SteadyDouble)
	->Iterations(MEASURE_CNT);
BENCHMARK_REGISTER_F(TimeElapsedMeasureFixture, SteadyInt)
	->Iterations(MEASURE_CNT);

BENCHMARK_REGISTER_F(TimeElapsedMeasureFixture, HighResolutionDouble)
	->Iterations(MEASURE_CNT);
BENCHMARK_REGISTER_F(TimeElapsedMeasureFixture, HighResolutionInt)
	->Iterations(MEASURE_CNT);

BENCHMARK_REGISTER_F(TimeElapsedMeasureFixture, timespec_int)
	->Iterations(MEASURE_CNT);

BENCHMARK_MAIN();
