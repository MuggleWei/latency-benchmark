#include "glog/logging.h"
#include "muggle/muggle_cc.h"

#include "latency_common/thread_trans_config.h"
#include "latency_common/thread_trans_reports.h"

typedef bool (*fn_write)(MuggleRingBuffer *ring_buffer, void *data, size_t len);
typedef void* (*fn_read)(MuggleRingBuffer *ring_buffer, int64_t idx);

struct Package
{
	int64_t idx;
	struct timespec ts;
};

struct MissingPkg
{
	int64_t idx;
	int64_t cursor;
};

void init_glog()
{
	fLI::FLAGS_max_log_size = 100;
	fLI::FLAGS_logbufsecs = 0;
	fLS::FLAGS_log_dir = "./log";
#if WIN32
	CreateDirectoryA("./log/", NULL);
#else
	mkdir("./log/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
	google::SetLogDestination(google::GLOG_INFO, "./log/thread_trans_ringbuffer.INFO.");
	google::SetLogDestination(google::GLOG_WARNING, "./log/thread_trans_ringbuffer.WARNING.");
	google::SetLogDestination(google::GLOG_ERROR, "./log/thread_trans_ringbuffer.ERROR.");
	google::InitGoogleLogging("");
}

void fn_consumer(int64_t *elapsed_array, ThreadTransConfig *config, MuggleRingBuffer *ring_buf)
{
	fn_read fn = nullptr;
	if (config->spin_read)
	{
		fn = MuggleRingBufferSpinRead;
	}
	else
	{
		fn = MuggleRingBufferYieldRead;
	}

	int64_t cnt = config->loop * config->cnt_per_loop;
	MissingPkg *missing_pkg_array = (MissingPkg*)malloc(sizeof(MissingPkg) * (1 + cnt / ring_buf->capacity));
	int64_t missing_idx = 0;

	struct timespec cur;
	int64_t idx = 0;
	while (true)
	{
		Package *pkg = (Package*)(*fn)(ring_buf, idx);
		struct timespec *ts = &pkg->ts;
		timespec_get(&cur, TIME_UTC);
		int64_t elapsed = (cur.tv_sec - ts->tv_sec) * 1000000000 + cur.tv_nsec - ts->tv_nsec;
		elapsed_array[idx] = elapsed;

		if (idx != pkg->idx)
		{
			missing_pkg_array[missing_idx].idx = idx;
			missing_pkg_array[missing_idx].cursor = pkg->idx;
			idx = pkg->idx;
			++missing_idx;
		}

		if (++idx == cnt)
		{
			break;
		}
	}

	LOG(INFO) << "consumer exit...";
	for (int64_t i = 0; i < missing_idx; ++i)
	{
		LOG(INFO) << "missing pkg: " << missing_pkg_array[i].idx << " - " << missing_pkg_array[i].cursor;
	}
	free(missing_pkg_array);
}

void fn_producer(ThreadTransConfig *config, MuggleRingBuffer *ring_buf)
{
	fn_write fn = nullptr;
	if (config->single_thread_write)
	{
		fn = MuggleRingBufferWriteSingleThread;
	}
	else
	{
		fn = MuggleRingBufferWrite;
	}

	Package pkg;
	for (int64_t i = 0; i < config->loop; ++i)
	{
		for (int64_t j = 0; j < config->cnt_per_loop; ++j)
		{
			pkg.idx = i * config->cnt_per_loop + j;
			timespec_get(&pkg.ts, TIME_UTC);
			(*fn)(ring_buf, &pkg, sizeof(pkg));
		}
		std::this_thread::sleep_for(std::chrono::microseconds(config->loop_interval_ms));
	}

	LOG(INFO) << "producer exit...";
}

void run(ThreadTransConfig *config)
{
	MuggleRingBuffer *ring_buf = (MuggleRingBuffer*)malloc(sizeof(MuggleRingBuffer));
	MuggleRingBufferInit(ring_buf, config->cnt_per_loop * 5, sizeof(Package));

	int64_t cnt = config->loop * config->cnt_per_loop;
	int64_t *elapsed_array = (int64_t*)malloc(sizeof(int64_t) * cnt);

	std::thread consumer(fn_consumer, elapsed_array, config, ring_buf);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	std::thread producer(fn_producer, config, ring_buf);
	consumer.join();
	producer.join();

	GenThreadTransReport(elapsed_array, cnt, config, "mugglecc_ringbuffer");

	free(elapsed_array);

	MuggleRingBufferDestory(ring_buf);
	free(ring_buf);
}

int main(int /*argc*/, char** /*argv*/)
{
	init_glog();
	LOG(INFO) << "start cpp thread trans ring buffer";

	ThreadTransConfig config;
	bool ret = LoadThreadTransConfig("thread_trans_conf.json", &config);
	if (!ret)
	{
		return 1;
	}

	run(&config);

	return 0;
}