#ifndef LATENCY_BENCHMARK_WS_SERVICE_H_
#define LATENCY_BENCHMARK_WS_SERVICE_H_

#include "uWS/uWS.h"
#include "latency_common/ws_config.h"

class WsService
{
public:
	WsService(WsConfig *config);
	~WsService();

	void onConnection(uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest &req);
	void onDisconnection(uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length);
	void onMessage(uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode);

private:
	WsConfig *config_;
	uWS::WebSocket<uWS::SERVER> *ws_;

	int64_t *elapsed_array_;
	int64_t max_len_;
	int64_t cnt_;
};

#endif