#include "thread_trans_reports.h"
#include <stdio.h>
#include <algorithm>
#include "glog/logging.h"

void WriteThreadTransReportHead(FILE *fp, ThreadTransConfig *config)
{
	fwrite("loop", 1, strlen("loop"), fp);
	fwrite(",", 1, strlen(","), fp);
	fwrite("cnt_per_loop", 1, strlen("cnt_per_loop"), fp);
	fwrite(",", 1, strlen(","), fp);
	fwrite("loop_interval_ms", 1, strlen("loop_interval_ms"), fp);
	fwrite(",", 1, strlen(","), fp);
	fwrite("name", 1, strlen("name"), fp);
	fwrite(",", 1, strlen(","), fp);
	for (int i = 0; i < 100; i += config->report_step)
	{
		char buf[16] = { 0 };
		snprintf(buf, sizeof(buf) - 1, "%d", i);
		fwrite(buf, 1, strlen(buf), fp);
		fwrite(",", 1, strlen(","), fp);
	}
	fwrite("100", 1, strlen("100"), fp);
	fwrite("\n", 1, strlen("\n"), fp);
}

void WriteThreadTransReport(FILE *fp, ThreadTransConfig *config, bool sorted, int64_t *elapsed, int64_t cnt, const char *name)
{
	char buf[256] = { 0 };
	snprintf(buf, sizeof(buf) - 1, "%d,%d,%d,%s,", config->loop, config->cnt_per_loop, config->loop_interval_ms, name);
	fwrite(buf, 1, strlen(buf), fp);

	for (int i = 0; i < 100; i += config->report_step)
	{
		int64_t idx = (int64_t)((i / 100.0) * cnt);
		snprintf(buf, sizeof(buf) - 1, "%lld", (long long)elapsed[idx]);
		fwrite(buf, 1, strlen(buf), fp);
		fwrite(",", 1, strlen(","), fp);
	}
	snprintf(buf, sizeof(buf) - 1, "%lld", (long long)elapsed[cnt - 1]);
	fwrite(buf, 1, strlen(buf), fp);
	fwrite("\n", 1, strlen("\n"), fp);
}


void GenThreadTransReport(int64_t *elapsed, int64_t cnt, ThreadTransConfig *config, const char *name)
{
	char file_name[64] = { 0 };
	snprintf(file_name, sizeof(file_name) - 1, "latency-threadtrans-%s-cpp.csv", name);

	FILE *fp = fopen(file_name, "w");
	if (fp == nullptr)
	{
		LOG(ERROR) << "failed open csv file: " << file_name;
		return;
	}

	WriteThreadTransReportHead(fp, config);
	WriteThreadTransReport(fp, config, false, elapsed, cnt, name);
	std::sort(elapsed, elapsed + cnt, std::less<int64_t>());
	WriteThreadTransReport(fp, config, false, elapsed, cnt, name);

	fclose(fp);
}
