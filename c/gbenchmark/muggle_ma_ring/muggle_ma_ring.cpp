#include "benchmark/benchmark.h"
#include "muggle/c/muggle_c.h"
#include <mutex>

#define CNT_ITER1 10000
#define MIN_TIME 2
#define CAP_SIZE 20000

typedef struct {
	struct timespec ts;
	uint32_t idx;
} data_t;

std::once_flag init_flag;

static FILE *fp = NULL;
void fn_write(muggle_ma_ring_t *ring, void *data)
{
	char buf[64];
	data_t *p = (data_t *)data;
	int n = snprintf(buf, sizeof(buf), "%llu, %u, %llu, %lu\n",
					 (unsigned long long)ring->tid, p->idx,
					 (unsigned long long)p->ts.tv_sec,
					 (unsigned long)p->ts.tv_nsec);
	fwrite(buf, 1, n, fp);
}

class MuggleMaRing : public benchmark::Fixture {
public:
	void SetUp(const benchmark::State &)
	{
		std::call_once(init_flag, []() {
			if (!muggle_path_exists("logs")) {
				muggle_os_mkdir("logs");
			}
			fp = fopen("logs/ma_ring.log", "w");
			if (fp == NULL) {
				exit(EXIT_FAILURE);
			}

			muggle_ma_ring_ctx_set_callback(fn_write);
			muggle_ma_ring_ctx_set_data_size(sizeof(data_t));
			muggle_ma_ring_ctx_set_capacity(CAP_SIZE);
			muggle_ma_ring_backend_run();
		});
		muggle_ma_ring_thread_ctx_init();
	}

	void TearDown(const benchmark::State &)
	{
		muggle_ma_ring_thread_ctx_cleanup();
	}
};

static muggle_thread_local uint32_t s_u32 = 0;
BENCHMARK_DEFINE_F(MuggleMaRing, Write)(benchmark::State &state)
{
	for (auto _ : state) {
		muggle_ma_ring_t *ring = muggle_ma_ring_thread_ctx_get();
		data_t *data = (data_t *)muggle_ma_ring_alloc(ring);
		muggle_realtime_get(data->ts);
		data->idx = s_u32++;
		muggle_ma_ring_move(ring);
	}
}

BENCHMARK_REGISTER_F(MuggleMaRing, Write)
	->Threads(1)->MinTime(MIN_TIME);
BENCHMARK_REGISTER_F(MuggleMaRing, Write)
	->Threads(2)->MinTime(MIN_TIME);
BENCHMARK_REGISTER_F(MuggleMaRing, Write)
	->Threads(4)->MinTime(MIN_TIME);
BENCHMARK_REGISTER_F(MuggleMaRing, Write)
	->Threads(8)->MinTime(MIN_TIME);

BENCHMARK_REGISTER_F(MuggleMaRing, Write)
	->Threads(1)
	->Iterations(CNT_ITER1)
	->Repetitions(5);
BENCHMARK_REGISTER_F(MuggleMaRing, Write)
	->Threads(2)
	->Iterations(CNT_ITER1)
	->Repetitions(5);
BENCHMARK_REGISTER_F(MuggleMaRing, Write)
	->Threads(4)
	->Iterations(CNT_ITER1)
	->Repetitions(5);
BENCHMARK_REGISTER_F(MuggleMaRing, Write)
	->Threads(8)
	->Iterations(CNT_ITER1)
	->Repetitions(5);

BENCHMARK_MAIN();
