#include "ws_reports.h"
#include <stdio.h>
#include <algorithm>
#include "glog/logging.h"

void WriteWsReportHead(FILE *fp, WsConfig *config)
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
		char buf[16] = { 0 };
		snprintf(buf, sizeof(buf) - 1, "%d", i);
		fwrite(buf, 1, strlen(buf), fp);
		fwrite(",", 1, strlen(","), fp);
	}
	fwrite("100", 1, strlen("100"), fp);
	fwrite("\n", 1, strlen("\n"), fp);
}

void WriteWsReport(FILE *fp, WsConfig *config, bool sorted, int64_t *elapsed, int64_t cnt)
{
	char buf[256] = { 0 };
	snprintf(buf, sizeof(buf) - 1, "%s,%d,%d,%d,%d,%s,", sorted ? "true" : "false", config->loop, config->cnt_per_loop, config->loop_interval_ms, (int)cnt, config->dir);
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

void GenWsReport(int64_t *elapsed, int64_t cnt, WsConfig *config)
{
	char file_name[64] = { 0 };
	snprintf(file_name, sizeof(file_name) - 1, "latency-ws-%s-cpp.csv", config->dir);

	FILE *fp = fopen(file_name, "w");
	if (fp == nullptr)
	{
		LOG(ERROR) << "failed open csv file: " << file_name;
		return;
	}

	WriteWsReportHead(fp, config);
	WriteWsReport(fp, config, false, elapsed, cnt);
	std::sort(elapsed, elapsed + cnt);
	WriteWsReport(fp, config, true, elapsed, cnt);

	fclose(fp);
}