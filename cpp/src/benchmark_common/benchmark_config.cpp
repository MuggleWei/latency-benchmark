#include "benchmark_config.h"
#include <iostream>
#include "rapidjson/document.h"

bool load_benchmark_config(const char *path, muggle::BenchmarkConfig *config)
{
	FILE *fp = nullptr;
	char *buf = nullptr;

	fp = fopen(path, "rb");
	if (fp == nullptr) {
		std::cerr << "failed read config file: " << path << std::endl;
		return false;
	}

	bool ret = true;
	try {
		fseek(fp, 0, SEEK_END);
		long cnt = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		buf = (char*)malloc((size_t)cnt + 1);
		memset(buf, 0, cnt + 1);
		auto read_cnt = fread(buf, 1, cnt, fp);
		if ((long)read_cnt != cnt) {
			throw(std::runtime_error("failed read config file"));
		}

		rapidjson::Document doc;
		doc.Parse(buf);

		if (doc.HasParseError())
		{
			std::cerr << "parse json error: " << doc.GetParseError() << std::endl;
			throw(std::runtime_error("failed parse json from config file"));
		}

		if (doc.HasMember("loop") && doc["loop"].IsInt())
		{
			config->loop = doc["loop"].GetInt();
		}
		else
		{
			throw(std::runtime_error("can't find 'loop' in config file"));
		}

		if (doc.HasMember("cnt_per_loop") && doc["cnt_per_loop"].IsInt())
		{
			config->cnt_per_loop = doc["cnt_per_loop"].GetInt();
		}
		else
		{
			throw(std::runtime_error("can't find 'cnt_per_loop' in config file"));
		}

		if (doc.HasMember("loop_interval_ms") && doc["loop_interval_ms"].IsInt())
		{
			config->loop_interval_ms = doc["loop_interval_ms"].GetInt();
		}
		else
		{
			throw(std::runtime_error("can't find 'loop_interval_ms' in config file"));
		}

		if (doc.HasMember("report_step") && doc["report_step"].IsInt())
		{
			config->report_step = doc["report_step"].GetInt();
		}
		else
		{
			throw(std::runtime_error("can't find 'report_step' in config file"));
		}
	}
	catch (std::runtime_error e)
	{
		std::cerr << e.what() << std::endl;
		ret = false;
	}
	catch (std::exception e)
	{
		std::cerr << e.what() << std::endl;
		ret = false;
	}

	if (buf) {
		free(buf);
	}

	if (fp) {
		fclose(fp);
	}

	return ret;
}
