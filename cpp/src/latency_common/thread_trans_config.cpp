#include "thread_trans_config.h"
#include "rapidjson/document.h"
#include "glog/logging.h"

bool LoadThreadTransConfig(const char *path, ThreadTransConfig *config)
{
	FILE *fp = nullptr;
	char *buf = nullptr;

	fp = fopen(path, "rb");
	if (fp == nullptr) {
		LOG(ERROR) << "failed read config file: " << path;
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
		if (read_cnt != cnt) {
			throw(std::runtime_error("failed read config file"));
		}

		rapidjson::Document doc;
		doc.Parse(buf);

		if (doc.HasParseError())
		{
			LOG(ERROR) << "parse json error: " << doc.GetParseError();
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
		LOG(ERROR) << e.what();
		ret = false;
	}
	catch (std::exception e)
	{
		LOG(ERROR) << e.what();
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