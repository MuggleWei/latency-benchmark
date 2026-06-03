#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

#if MUGGLE_PLATFORM_LINUX

	#define SOCK_WRITE_IDX 1
	#define SOCK_READ_IDX 0

static muggle_spinlock_t spinlock;

int socketpair_write(void *user_args, void *data)
{
	int *fds = (int *)user_args;

	muggle_spinlock_lock(&spinlock);

	muggle_atomic_thread_fence(muggle_memory_order_release);
	int n = write(fds[SOCK_WRITE_IDX], &data, sizeof(void *));

	muggle_spinlock_unlock(&spinlock);
	return n == -1 ? -1 : 0;
}

void *socketpair_read(void *user_args, int consumer_id)
{
	MUGGLE_UNUSED(consumer_id);
	int *fds = (int *)user_args;

	void *data = NULL;
	while (read(fds[SOCK_READ_IDX], &data, sizeof(void *)) != sizeof(void *))
		;

	muggle_atomic_thread_fence(muggle_memory_order_acquire);
	return data;
}

void producer_complete_cb(muggle_benchmark_config_t *config, void *user_args)
{
	MUGGLE_UNUSED(config);

	static muggle_benchmark_thread_message_t end_msg;
	memset(&end_msg, 0, sizeof(end_msg));
	end_msg.id = UINT64_MAX;

	socketpair_write(user_args, &end_msg);
}

void benchmark_socketpair(muggle_benchmark_config_t *config, const char *name)
{
	// prepare socketpair and set read non-blocking
	int fds[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == -1) {
		MUGGLE_LOG_ERROR("failed create socketpair");
		return;
	}

	muggle_socket_set_nonblock(fds[SOCK_READ_IDX], 1);

	// prepare mutex
	muggle_spinlock_init(&spinlock);

	// initialize thread transfer benchmark
	muggle_benchmark_thread_trans_t benchmark;
	muggle_benchmark_thread_trans_init(&benchmark, config, (void *)&fds[0],
									   socketpair_write, socketpair_read,
									   producer_complete_cb);

	// run benchmark
	muggle_benchmark_thread_trans_run(&benchmark);

	// generate report
	muggle_benchmark_thread_trans_gen_report(&benchmark, name);

	// destroy benchmark
	muggle_benchmark_thread_trans_destroy(&benchmark);

	// close sockets
	close(fds[0]);
	close(fds[1]);
}

int main(int argc, char *argv[])
{
	// initialize log
	muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO);

	// initialize benchmark config
	muggle_benchmark_config_t config;
	muggle_benchmark_config_parse_cli(&config, argc, argv);

	config.producer = 0;

	muggle_benchmark_config_output(&config);

	// channel must guarantee only one reader
	if (config.consumer != 1) {
		MUGGLE_LOG_ERROR("user must guarantee only one message consumeer");
		exit(EXIT_FAILURE);
	}

	int flags = 0;

	int hc = (int)muggle_thread_hardware_concurrency();
	if (hc <= 0) {
		hc = 2;
	}

	int producer_nums[] = { 1, 2, 4, hc / 2, hc, hc * 2, hc * 4 };
	char name[64];
	for (int i = 0; i < (int)(sizeof(producer_nums) / sizeof(producer_nums[0]));
		 i++) {
		int num_producer = producer_nums[i];
		config.producer = num_producer;

		memset(name, 0, sizeof(name));
		snprintf(name, sizeof(name), "socketpair_%d_w", num_producer);

		MUGGLE_LOG_INFO(
			"--------------------------------------------------------");
		MUGGLE_LOG_INFO("run pipe - %d write", num_producer);
		benchmark_socketpair(&config, name);
	}

	return 0;
}

#else

int main()
{
	muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO);

	MUGGLE_LOG_ERROR("support linux only");

	return 0;
}

#endif /* MUGGLE_PLATFORM_LINUX */
