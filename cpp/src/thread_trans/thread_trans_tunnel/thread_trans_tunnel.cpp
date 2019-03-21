#if WIN32
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <time.h>
#include <chrono>

#include "glog/logging.h"
#include "muggle/muggle_cc.h"

#include "latency_common/thread_trans_config.h"
#include "latency_common/thread_trans_reports.h"

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
	google::SetLogDestination(google::GLOG_INFO, "./log/thread_trans_tunnel.INFO.");
	google::SetLogDestination(google::GLOG_WARNING, "./log/thread_trans_tunnel.WARNING.");
	google::SetLogDestination(google::GLOG_ERROR, "./log/thread_trans_tunnel.ERROR.");
	google::InitGoogleLogging("");
}

void fn_consumer(int64_t *elapsed_array, ThreadTransConfig *config, muggle::Tunnel<struct timeval> &tunnel)
{
	std::queue<struct timeval> queue;
	struct timeval cur;
	int64_t cnt = config->loop * config->cnt_per_loop;
	int64_t idx = 0;
	while (true)
	{
		tunnel.Read(queue);
		while (queue.size() > 0)
		{
			gettimeofday(&cur, nullptr);
			struct timeval &tv = queue.front();
			int64_t elapsed = (cur.tv_sec - tv.tv_sec) * 1000000 + cur.tv_usec - tv.tv_usec;
			if (idx >= cnt)
			{
				LOG(INFO) << "fuck";
				continue;
			}
			elapsed_array[idx++] = elapsed;
			queue.pop();
		}

		if (idx == cnt)
		{
			break;
		}
	}
}

void fn_producer(ThreadTransConfig *config, muggle::Tunnel<struct timeval> &tunnel)
{
	struct timeval tv;
	for (int64_t i = 0; i < config->loop; ++i)
	{
		for (int64_t j = 0; j < config->cnt_per_loop; ++j)
		{
			gettimeofday(&tv, nullptr);
			tunnel.Write(tv);
		}
		std::this_thread::sleep_for(std::chrono::microseconds(config->loop_interval_ms));
	}
}

void run(ThreadTransConfig *config)
{
	muggle::Tunnel<struct timeval> tunnel;
	int64_t cnt = config->loop * config->cnt_per_loop;
	int64_t *elapsed_array = (int64_t*)malloc(sizeof(int64_t) * cnt);

	std::thread consumer(fn_consumer, elapsed_array, config, std::ref(tunnel));
	std::this_thread::sleep_for(std::chrono::seconds(1));

	std::thread producer(fn_producer, config, std::ref(tunnel));;
	consumer.join();
	producer.join();

	GenThreadTransReport(elapsed_array, cnt, config, "mugglecc_tunnel");

	free(elapsed_array);
}

int main(int /*argc*/, char** /*argv*/)
{
	init_glog();
	LOG(INFO) << "start cpp thread trans tunnel";

	ThreadTransConfig config;
	bool ret = LoadThreadTransConfig("thread_trans_conf.json", &config);
	if (!ret)
	{
		return 1;
	}

	run(&config);

	return 0;
}
