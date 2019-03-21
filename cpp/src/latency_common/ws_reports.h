#ifndef LATENCY_COMMON_WS_REPORTS_H_
#define LATENCY_COMMON_WS_REPORTS_H_

#include <stdint.h>
#include "ws_config.h"

void GenWsReport(int64_t *elapsed, int64_t cnt, WsConfig *config);

#endif