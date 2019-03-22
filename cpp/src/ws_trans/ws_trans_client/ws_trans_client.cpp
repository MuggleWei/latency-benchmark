#include <stdint.h>
#include <time.h>
#include <array>
#include <exception>
#include <chrono>

#include "glog/logging.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "uWS.h"
#include "muggle/muggle_cc.h"

#if WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "latency_common/ws_config.h"
#include "ws_trans/ws_trans_handle/ws_handle.h"

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
	google::SetLogDestination(google::GLOG_INFO, "./log/ws_trans_client_cpp.INFO.");
	google::SetLogDestination(google::GLOG_WARNING, "./log/ws_trans_client_cpp.WARNING.");
	google::SetLogDestination(google::GLOG_ERROR, "./log/ws_trans_client_cpp.ERROR.");
	google::InitGoogleLogging("");
}

void run_stc(WsConfig *config)
{
	uWS::Hub hub;
	WsHandle handle(config);
	char addr[128];
	snprintf(addr, sizeof(addr) - 1, "ws://%s:%d%s", config->host, config->port, config->url);

	hub.onMessage([&](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode) {
		handle.onMessage(ws, message, length, opCode);
	});
	hub.onDisconnection([&](uWS::WebSocket<uWS::CLIENT> *ws, int code, char *message, size_t length) {
		handle.onDisconnection(ws, code, message, length);
	});

	hub.connect(addr, nullptr);
	hub.run();
}

void run_cts(WsConfig *config)
{
	uWS::Hub hub;
	char addr[128];
	snprintf(addr, sizeof(addr) - 1, "ws://%s:%d%s", config->host, config->port, config->url);

	hub.onConnection([&](uWS::WebSocket<uWS::CLIENT> *ws, uWS::HttpRequest req) {
		auto t1 = std::chrono::high_resolution_clock::now();

		LOG(INFO) << "start send message\n"
			<< "loop: " << config->loop << "\n"
			<< "cnt_per_loop: " << config->cnt_per_loop;

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

				ws->send(s.GetString(), s.GetSize(), uWS::OpCode::TEXT);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(config->loop_interval_ms));
		}

		auto t2 = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> total_elapsed_ms = t2 - t1;
		LOG(INFO) << "total elapsed: " << total_elapsed_ms.count() << " ms";

		hub.getDefaultGroup<uWS::CLIENT>().close();
	});

	hub.connect(addr, nullptr);
	hub.run();
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

	if (strncmp(config.dir, "stc", 3) == 0)
	{
		run_stc(&config);
	}
	else
	{
		run_cts(&config);
	}
	

	return 0;
}
