#ifndef LATENCY_COMMON_THREAD_TRANS_REPORTS_H_
#define LATENCY_COMMON_THREAD_TRANS_REPORTS_H_

#include <stdint.h>
#include "latency_common/thread_trans_config.h"

void GenThreadTransReport(int64_t *elapsed, int64_t cnt, ThreadTransConfig *config, const char *name);

#endif