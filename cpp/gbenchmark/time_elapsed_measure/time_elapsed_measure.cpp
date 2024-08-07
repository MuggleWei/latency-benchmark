#include <chrono>
#include <stdint.h>
#include "benchmark/benchmark.h"

#if __linux__ && defined(__x86_64__)
	#include <x86intrin.h>
#endif

#define MEASURE_CNT 100000

class TimeElapsedMeasureFixture : public benchmark::Fixture {
public:
	virtual void SetUp(const benchmark::State &) override
	{
		double_arr = (double *)malloc(sizeof(double) * MEASURE_CNT);
		int64_arr = (int64_t *)malloc(sizeof(int64_t) * MEASURE_CNT);
		uint64_arr = (uint64_t *)malloc(sizeof(uint64_t) * MEASURE_CNT);
		idx = 0;

		steady_start = std::chrono::steady_clock::now();
		high_resolution_start = std::chrono::high_resolution_clock::now();
		timespec_get(&ts_start, TIME_UTC);
#ifdef __linux__
		clock_gettime(CLOCK_MONOTONIC, &monotonic_clock_start);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &process_clock_start);
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &thread_clock_start);

	#ifdef __x86_64__
		rdtsc_start = __rdtsc();
	#endif // __x86_64__

#endif
	}

	virtual void TearDown(const benchmark::State &) override
	{
		free(double_arr);
		free(int64_arr);
		free(uint64_arr);
	}

public:
	double *double_arr;
	int64_t *int64_arr;
	uint64_t *uint64_arr;
	uint32_t idx;

	std::chrono::steady_clock::time_point steady_start;
	std::chrono::high_resolution_clock::time_point high_resolution_start;
	struct timespec ts_start;
	struct timespec monotonic_clock_start;
	struct timespec process_clock_start;
	struct timespec thread_clock_start;
	uint64_t rdtsc_start;
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
	// make sure my duration type is the same as default, so it's don't need to
	// real convert duration type in runtime
	typedef std::chrono::duration<int64_t, std::nano> steady_int64_nano_duration;
	static_assert(std::is_same<steady_int64_nano_duration,
							   std::chrono::steady_clock::duration>::value);

	for (auto _ : state) {
		auto cur = std::chrono::steady_clock::now();
		steady_int64_nano_duration diff =
			std::chrono::duration_cast<steady_int64_nano_duration>(
				cur - steady_start);
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
	// make sure my duration type is the same as default, so it's don't need to
	// real convert duration type in runtime
	typedef std::chrono::duration<int64_t, std::nano>
		high_resolution_int64_nano_duration;
	static_assert(
		std::is_same<high_resolution_int64_nano_duration,
					 std::chrono::high_resolution_clock::duration>::value);

	for (auto _ : state) {
		auto cur = std::chrono::high_resolution_clock::now();
		high_resolution_int64_nano_duration diff =
			std::chrono::duration_cast<high_resolution_int64_nano_duration>(
				cur - high_resolution_start);
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

#ifdef __linux__

BENCHMARK_DEFINE_F(TimeElapsedMeasureFixture, clock_gettime_MONOTONIC)
(benchmark::State &state)
{
	for (auto _ : state) {
		struct timespec cur;
		clock_gettime(CLOCK_MONOTONIC, &cur);
		int64_arr[idx++] =
			(cur.tv_sec - monotonic_clock_start.tv_sec) * 1000000000 +
			cur.tv_nsec - monotonic_clock_start.tv_nsec;
	}
}

BENCHMARK_DEFINE_F(TimeElapsedMeasureFixture, clock_gettime_PROCESS_CPUTIME)
(benchmark::State &state)
{
	for (auto _ : state) {
		struct timespec cur;
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cur);
		int64_arr[idx++] =
			(cur.tv_sec - process_clock_start.tv_sec) * 1000000000 +
			cur.tv_nsec - process_clock_start.tv_nsec;
	}
}

BENCHMARK_DEFINE_F(TimeElapsedMeasureFixture, clock_gettime_THREAD_CPUTIME)
(benchmark::State &state)
{
	for (auto _ : state) {
		struct timespec cur;
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &cur);
		int64_arr[idx++] =
			(cur.tv_sec - thread_clock_start.tv_sec) * 1000000000 +
			cur.tv_nsec - thread_clock_start.tv_nsec;
	}
}

	#ifdef __x86_64__

BENCHMARK_DEFINE_F(TimeElapsedMeasureFixture, rdtsc)
(benchmark::State &state)
{
	for (auto _ : state) {
		uint64_t cur = __rdtsc();
		uint64_arr[idx++] = cur - rdtsc_start;
	}
}

	#endif // __x86_64__

#endif

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

#ifdef __linux__

BENCHMARK_REGISTER_F(TimeElapsedMeasureFixture, clock_gettime_MONOTONIC)
	->Iterations(MEASURE_CNT);
BENCHMARK_REGISTER_F(TimeElapsedMeasureFixture, clock_gettime_PROCESS_CPUTIME)
	->Iterations(MEASURE_CNT);
BENCHMARK_REGISTER_F(TimeElapsedMeasureFixture, clock_gettime_THREAD_CPUTIME)
	->Iterations(MEASURE_CNT);

	#ifdef __x86_64__

BENCHMARK_REGISTER_F(TimeElapsedMeasureFixture, rdtsc)->Iterations(MEASURE_CNT);

	#endif // __x86_64__

#endif

BENCHMARK_MAIN();
