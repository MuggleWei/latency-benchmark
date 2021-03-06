#include "ws_config.h"
#include "rapidjson/document.h"
#include "glog/logging.h"

bool LoadWsConfig(const char *path, WsConfig *config)
{
	FILE *fp = nullptr;
	char *buf = nullptr;

	fp = fopen(path, "rb");
	if (fp == nullptr) {
		LOG(ERROR) << "failed read ws config file: " << path;
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
			throw(std::runtime_error("failed parse json from config file"));
		}

		if (doc.HasMember("host") && doc["host"].IsString())
		{
			strncpy(config->host, doc["host"].GetString(), sizeof(config->host)-1);
		}
		else
		{
			throw(std::runtime_error("can't find 'host' in config file"));
		}

		if (doc.HasMember("port") && doc["port"].IsInt())
		{
			config->port = doc["port"].GetInt();
		}
		else
		{
			throw(std::runtime_error("can't find 'port' in config file"));
		}

		if (doc.HasMember("url") && doc["url"].IsString())
		{
			strncpy(config->url, doc["url"].GetString(), sizeof(config->url) - 1);
		}
		else
		{
			throw(std::runtime_error("can't find 'url' in config file"));
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

		if (doc.HasMember("dir") && doc["dir"].IsString())
		{
			strncpy(config->dir, doc["dir"].GetString(), sizeof(config->host)-1);
		}
		else
		{
			throw(std::runtime_error("can't find 'dir' in config file"));
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
