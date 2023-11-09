#include <cstdlib>
#include <stdio.h>
#include <atomic>
#include <thread>
#include <chrono>
#if __linux__
	#ifndef _GNU_SOURCE
		#define _GNU_SOURCE
	#endif // !_GNU_SOURCE
	#include <sched.h>
#endif

struct benchmark_item {
	unsigned int cpu;
	unsigned int node;
	double elapsed;
};

void run(benchmark_item *p_item)
{
#if __linux__
	getcpu(&p_item->cpu, &p_item->node);
#endif

	std::atomic<int> ival{ 1 };

	// int v_max = 0;
	// int *p_max = &v_max;
	// std::thread reader_thread([&ival, p_max] {
	//     int v = 0;
	//     while (true) {
	//         v = ival.load(std::memory_order_relaxed);
	//         if (v > *p_max) {
	//             *p_max = v;
	//         }
	//         if (v == 0) {
	//             break;
	//         }
	//     }
	// });

	const int warnup_cnt = 1000000;
	for (int i = 0; i < warnup_cnt; i++) {
		ival.fetch_add(1, std::memory_order_relaxed);
	}

	const int cnt = 10000;
	const auto start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < cnt; i++) {
		ival.fetch_add(1, std::memory_order_relaxed);
	}

	const auto end = std::chrono::high_resolution_clock::now();

	double elapsed =
		std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
			.count();

	ival.store(0, std::memory_order_relaxed);

	// reader_thread.join();

	p_item->elapsed = elapsed / cnt;
}

int cmp_benchmark_item(const void *a, const void *b)
{
	benchmark_item *p1 = (benchmark_item *)a;
	benchmark_item *p2 = (benchmark_item *)b;
	return p1->elapsed - p2->elapsed;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "failed launch, arc < 2!\n");
		fprintf(stderr, "Usage: %s <num-thread>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int cnt = atoi(argv[1]);

	std::thread **threads = (std::thread **)malloc(sizeof(std::thread *) * cnt);
	benchmark_item *results =
		(benchmark_item *)malloc(sizeof(benchmark_item) * cnt);
	for (int i = 0; i < cnt; i++) {
		benchmark_item *p = results + i;
		threads[i] = new std::thread([p] { run(p); });
	}
	for (int i = 0; i < cnt; i++) {
		threads[i]->join();
		delete threads[i];
	}
	free(threads);

	fprintf(stdout, "--------------------------------\n");
	fprintf(stdout, "# fetchadd thread_local\n");
	for (int i = 0; i < cnt; i++) {
#if __linux__
		fprintf(stdout,
				"%d|cpu: %u, node: %u, elapsed: %f ns\n",
				i, results[i].cpu, results[i].node, results[i].elapsed);
#else
		fprintf(stdout, "fetchadd thread_local %d|elapsed: %f ns\n", i,
				results[i].elapsed);
#endif
	}

	double sum = 0.0;
	for (int i = 0; i < cnt; i++) {
		sum += results[i].elapsed;
	}

	qsort(results, cnt, sizeof(benchmark_item), cmp_benchmark_item);
	fprintf(stdout, ">>\n");
	fprintf(stdout, "min: %f ns\n", results[0].elapsed);
	fprintf(stdout, "max: %f ns\n", results[cnt - 1].elapsed);
	fprintf(stdout, "median: %f ns\n", results[cnt / 2].elapsed);
	fprintf(stdout, "mean: %f ns\n", sum / cnt);

	free(results);

	return 0;
}
