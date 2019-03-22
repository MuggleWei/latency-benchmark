#include "ws_handle.h"
#include "glog/logging.h"
#include "muggle/muggle_cc.h"
#include "latency_common/ws_reports.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

WsHandle::WsHandle(WsConfig *config)
	: config_(config)
	, elapsed_array_(nullptr)
	, cnt_(0)
{
	max_len_ = config->loop * config->cnt_per_loop;
	elapsed_array_ = (int64_t*)malloc(sizeof(int64_t) * max_len_);
	cnt_ = 0;
}

WsHandle::~WsHandle()
{
	if (elapsed_array_ != nullptr)
	{
		free(elapsed_array_);
	}
}

void WsHandle::onConnection(uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest &req)
{
	LOG(INFO) << "ws connection: " << ws->getAddress().address << ":" << ws->getAddress().port << ", url: " << req.getUrl().toString() << std::endl;
	if (req.getUrl().toString() != config_->url)
	{
		ws->close();
	}
	else
	{
		ws_ = ws;
	}
}
void WsHandle::onDisconnection(uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length)
{
	if (strncmp(config_->dir, "cts", 3) == 0)
	{
		char file_name[64] = { 0 };
		snprintf(file_name, sizeof(file_name) - 1, "latency-ws-%s-cpp.csv", config_->dir);
		FILE *fp = fopen(file_name, "w");
		if (fp == nullptr)
		{
			LOG(ERROR) << "failed open csv file";
		}

		GenWsReport(elapsed_array_, cnt_, config_);
	}
}
void WsHandle::onMessage(uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode)
{
	onMessage(message, length);
}

void WsHandle::onDisconnection(uWS::WebSocket<uWS::CLIENT> *ws, int code, char *message, size_t length)
{
	if (strncmp(config_->dir, "stc", 3) == 0)
	{
		char file_name[64] = { 0 };
		snprintf(file_name, sizeof(file_name) - 1, "latency-ws-%s-cpp.csv", config_->dir);
		FILE *fp = fopen(file_name, "w");
		if (fp == nullptr)
		{
			LOG(ERROR) << "failed open csv file";
		}

		GenWsReport(elapsed_array_, cnt_, config_);
	}
}
void WsHandle::onMessage(uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode)
{
	onMessage(message, length);
}

void WsHandle::onMessage(char *message, size_t length)
{
	if (cnt_ > max_len_ - 1)
	{
		return;
	}

	struct timeval tv;
	gettimeofday(&tv, nullptr);

	try
	{
		rapidjson::Document doc;
		doc.Parse(message, length);
		if (doc.HasParseError())
		{
			LOG(ERROR) << "rapidjson get parse error " << doc.GetParseError();
			return;
		}

		int64_t s = doc["s"].GetInt64();
		int64_t us = doc["us"].GetInt64();
		int64_t elapsed = (tv.tv_sec - s) * 1000000 + tv.tv_usec - us;
		elapsed_array_[cnt_++] = elapsed;
	}
	catch (std::exception &e)
	{
		LOG(ERROR) << e.what();
	}
}