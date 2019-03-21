#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include <stdint.h>
#include <array>
#include <exception>
#include <chrono>

#include "glog/logging.h"
#include "App.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "latency_common/ws_config.h"

#define MAX_LEN 10000000

static std::array<int64_t, MAX_LEN> elapsed_array;

struct PeerInfo
{
	int id;
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
	google::SetLogDestination(google::GLOG_INFO, "./log/ws_trans_server_cpp.INFO.");
	google::SetLogDestination(google::GLOG_WARNING, "./log/ws_trans_server_cpp.WARNING.");
	google::SetLogDestination(google::GLOG_ERROR, "./log/ws_trans_server_cpp.ERROR.");
	google::InitGoogleLogging("");
}

void write_report_head(FILE *fp, WsConfig *config)
{
	fwrite("sorted", 1, strlen("sorted"), fp);
	fwrite(",", 1, strlen(","), fp);
	fwrite("loop", 1, strlen("loop"), fp);
	fwrite(",", 1, strlen(","), fp);
	fwrite("cnt_per_loop", 1, strlen("cnt_per_loop"), fp);
	fwrite(",", 1, strlen(","), fp);
	fwrite("loop_interval_ms", 1, strlen("loop_interval_ms"), fp);
	fwrite(",", 1, strlen(","), fp);
	fwrite("pkg_cnt", 1, strlen("pkg_cnt"), fp);
	fwrite(",", 1, strlen(","), fp);
	fwrite("dir", 1, strlen("dir"), fp);
	fwrite(",", 1, strlen(","), fp);
	for (int i = 0; i < 100; i += config->report_step)
	{
		char buf[16] = {0};
		snprintf(buf, sizeof(buf)-1, "%d", i);
		fwrite(buf, 1, strlen(buf), fp);
		fwrite(",", 1, strlen(","), fp);
	}
	fwrite("100", 1, strlen("100"), fp);
	fwrite("\n", 1, strlen("\n"), fp);
}

void write_report(FILE *fp, WsConfig *config, bool sorted, std::array<int64_t, MAX_LEN> &elapsed, int64_t cnt)
{
	char buf[256] = {0};
	snprintf(buf, sizeof(buf)-1, "%s,%d,%d,%d,%d,%s,", sorted ? "true":"false", config->loop, config->cnt_per_loop, config->loop_interval_ms, (int)cnt, config->dir);
	fwrite(buf, 1, strlen(buf), fp);

	for (int i = 0; i < 100; i += config->report_step)
	{
		int64_t idx = (int64_t)((i / 100.0) * cnt);
		snprintf(buf, sizeof(buf)-1, "%lld", (long long)elapsed[idx]);
		fwrite(buf, 1, strlen(buf), fp);
		fwrite(",", 1, strlen(","), fp);
	}
	snprintf(buf, sizeof(buf)-1, "%lld", (long long)elapsed[cnt-1]);
	fwrite(buf, 1, strlen(buf), fp);
	fwrite("\n", 1, strlen("\n"), fp);
}

void run(WsConfig *config)
{
	int64_t cnt = 0;

	uWS::App().ws<PeerInfo>("/ws", {
        .compression = uWS::DISABLED,
        .maxPayloadLength = 16 * 1024,
        .idleTimeout = 120,
        .open = [&](auto *ws, auto *req){
			if (strncmp(config->dir, "stc", 3) == 0) {
				std::thread th([&]{
					auto t1 = std::chrono::high_resolution_clock::now();

					std::cout << "start send message\n"
						<< "loop: " << config->loop << "\n"
						<< "cnt_per_loop: " << config->cnt_per_loop << std::endl;

					for (int i = 0; i < config->loop; ++i) {
						for (int j = 0; j < config->cnt_per_loop; ++j) {
							struct timeval tv;
							gettimeofday(&tv, nullptr);

							rapidjson::StringBuffer s;
							rapidjson::Writer<rapidjson::StringBuffer> writer(s);
							writer.StartObject();
							writer.Key("s");
							writer.Int64(tv.tv_sec);
							writer.Key("us");
							writer.Int64(tv.tv_usec);
							writer.EndObject();

							ws->send(s.GetString(), uWS::OpCode::TEXT);
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(config->loop_interval_ms));
					}

					auto t2 = std::chrono::high_resolution_clock::now();
					std::chrono::duration<double, std::milli> total_elapsed_ms = t2 - t1;
					LOG(INFO) << "total elapsed: " << total_elapsed_ms.count() << " ms";

					ws->close();
				});
				th.detach();
			}
		},
        .message = [&](auto *ws, std::string_view message, uWS::OpCode opCode) {
			if (cnt > MAX_LEN - 1) {
				return;
			}

			struct timeval tv;
			gettimeofday(&tv, nullptr);

			try
			{
				rapidjson::Document doc;
				doc.Parse(message.data(), message.size());
				if (doc.HasParseError())
				{
					LOG(ERROR) << "rapidjson get parse error " << doc.GetParseError();
					return;
				}

				int64_t s = doc["s"].GetInt64();
				int64_t us = doc["us"].GetInt64();
				int64_t elapsed = (tv.tv_sec - s) * 1000000 + tv.tv_usec - us;
				elapsed_array[cnt++] = elapsed;
			}
			catch (std::exception &e)
			{
				LOG(ERROR) << e.what();
			}
		},
        .drain = nullptr,
        .ping = nullptr,
        .pong = nullptr,
        .close = [&](auto *ws, int code, std::string_view message) {
			if (strncmp(config->dir, "cts", 3) == 0)
			{
				char file_name[64] = {0};
				snprintf(file_name, sizeof(file_name)-1, "latency-ws-%s-cpp.csv", config->dir);
				FILE *fp = fopen(file_name, "w");
				if (fp == nullptr)
				{
					LOG(ERROR) << "failed open csv file";
				}

				write_report_head(fp, config);
				write_report(fp, config, false, elapsed_array, cnt);
				std::sort(elapsed_array.begin(), elapsed_array.begin() + cnt);
				write_report(fp, config, true, elapsed_array, cnt);

				fclose(fp);
			}

			exit(0);
		}
	}).listen(config->host, config->port, [&](auto *token) {
		if (token) {
			LOG(INFO) << "listen on " << config->host << ":" << config->port;
		}
	}).run();
}

int main(int /*argc*/, char** /*argv*/)
{
	init_glog();
	LOG(INFO) << "start cpp ws_trans server";

	WsConfig config;
	bool ret = LoadWsConfig("ws_conf.json", &config);
	if (!ret)
	{
		return 1;
	}

	run(&config);

	return 0;
}
