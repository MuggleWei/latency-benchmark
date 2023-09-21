#include "benchmark/benchmark.h"
#include "muggle/c/muggle_c.h"

#define RING_CAPACITY 4096

typedef struct block
{
	union {
		void *data;
		MUGGLE_STRUCT_CACHE_LINE_PADDING(0);
	};
} block_t;

class ThreadTransWriteFixture : public benchmark::Fixture
{
public:
	void SetUp(const benchmark::State &)
	{
		for (int i = 0; i < RING_CAPACITY; i++)
		{
			datas[i] = i;
		}

		cursor = 0;
		next = 0;

		muggle_spinlock_init(&spinlock);
		muggle_mutex_init(&mtx);
#if MUGGLE_C_HAVE_SYNC_OBJ
		muggle_synclock_init(&sync);
#endif
	}

	void TearDown(const benchmark::State &)
	{
		muggle_mutex_destroy(&mtx);
	}

	void AsignInLock()
	{
		int idx = IDX_IN_POW_OF_2_RING(cursor, RING_CAPACITY);
		ring[idx].data = &datas[idx];
		muggle_atomic_store(&cursor, cursor + 1, muggle_memory_order_release);
	}

public:
	int datas[RING_CAPACITY];
	block_t ring[RING_CAPACITY];
	muggle_atomic_int cursor;
	muggle_atomic_int next;

	muggle_spinlock_t spinlock;
	muggle_mutex_t mtx;
#if MUGGLE_C_HAVE_SYNC_OBJ
	muggle_sync_t sync;
#endif
};

BENCHMARK_DEFINE_F(ThreadTransWriteFixture, Single)(benchmark::State &state)
{
	for (auto _ : state)
	{
		AsignInLock();
	}
}
BENCHMARK_REGISTER_F(ThreadTransWriteFixture, Single)->Threads(1);

BENCHMARK_DEFINE_F(ThreadTransWriteFixture, SpinLock)(benchmark::State &state)
{
	for (auto _ : state)
	{
		muggle_spinlock_lock(&spinlock);
		AsignInLock();
		muggle_spinlock_unlock(&spinlock);
	}
}
BENCHMARK_REGISTER_F(ThreadTransWriteFixture, SpinLock)->Threads(1);
BENCHMARK_REGISTER_F(ThreadTransWriteFixture, SpinLock)->Threads(2);
BENCHMARK_REGISTER_F(ThreadTransWriteFixture, SpinLock)->Threads(4);
BENCHMARK_REGISTER_F(ThreadTransWriteFixture, SpinLock)->Threads(8);

BENCHMARK_DEFINE_F(ThreadTransWriteFixture, Mutex)(benchmark::State &state)
{
	for (auto _ : state)
	{
		muggle_mutex_lock(&mtx);
		AsignInLock();
		muggle_mutex_unlock(&mtx);
	}
}
BENCHMARK_REGISTER_F(ThreadTransWriteFixture, Mutex)->Threads(1);
BENCHMARK_REGISTER_F(ThreadTransWriteFixture, Mutex)->Threads(2);
BENCHMARK_REGISTER_F(ThreadTransWriteFixture, Mutex)->Threads(4);
BENCHMARK_REGISTER_F(ThreadTransWriteFixture, Mutex)->Threads(8);

#if MUGGLE_C_HAVE_SYNC_OBJ

BENCHMARK_DEFINE_F(ThreadTransWriteFixture, Sync)(benchmark::State &state)
{
	for (auto _ : state)
	{
		muggle_synclock_lock(&sync);
		AsignInLock();
		muggle_synclock_unlock(&sync);
	}
}
BENCHMARK_REGISTER_F(ThreadTransWriteFixture, Sync)->Threads(1);
BENCHMARK_REGISTER_F(ThreadTransWriteFixture, Sync)->Threads(2);
BENCHMARK_REGISTER_F(ThreadTransWriteFixture, Sync)->Threads(4);
BENCHMARK_REGISTER_F(ThreadTransWriteFixture, Sync)->Threads(8);

#endif

BENCHMARK_DEFINE_F(ThreadTransWriteFixture, CAS)(benchmark::State &state)
{
	for (auto _ : state)
	{
		muggle_atomic_int pos = muggle_atomic_fetch_add(&next, 1, muggle_memory_order_relaxed);

		int idx = IDX_IN_POW_OF_2_RING(pos, RING_CAPACITY);
		ring[idx].data = &datas[idx];

		muggle_atomic_int cur_pos = pos;
		while (!muggle_atomic_cmp_exch_weak(&cursor, &cur_pos, pos + 1, muggle_memory_order_release)
			&& cur_pos != pos)
		{
			muggle_thread_yield();
			cur_pos = pos;
		}
	}
}
BENCHMARK_REGISTER_F(ThreadTransWriteFixture, CAS)->Threads(1);
BENCHMARK_REGISTER_F(ThreadTransWriteFixture, CAS)->Threads(2);
BENCHMARK_REGISTER_F(ThreadTransWriteFixture, CAS)->Threads(4);
BENCHMARK_REGISTER_F(ThreadTransWriteFixture, CAS)->Threads(8);

BENCHMARK_MAIN();
