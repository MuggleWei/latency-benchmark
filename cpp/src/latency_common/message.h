#ifndef LATENCY_COMMON_MESSAGE_H_
#define LATENCY_COMMON_MESSAGE_H_

#include <stdint.h>

struct TimestampBlock
{
	int64_t sec;
	int64_t usec;
};

#endif