#ifndef BTCLITE_LOGGING_H
#define BTCLITE_LOGGING_H

#include <array>
#include <atomic>
#include <string>
#include <map>

#include <glog/logging.h>

#include "Assert.h"

class Logging;

#define DEFAULT_LOG_LEVEL  "3"

#define LOGLEVEL_FATAL    0
#define LOGLEVEL_ERROR    1
#define LOGLEVEL_WARNING  2
#define LOGLEVEL_INFO     3
#define LOGLEVEL_DEBUG    4
#define LOGLEVEL_VERBOSE  5
#define LOGLEVEL_MAX      6

#define LOG_LOGLEVEL_FATAL   LOG(FATAL)
#define LOG_LOGLEVEL_ERROR   LOG(ERROR)
#define LOG_LOGLEVEL_WARNING LOG(WARNING)
#define LOG_LOGLEVEL_INFO    VLOG(Logging::VERBOSE0)
#define LOG_LOGLEVEL_DEBUG   VLOG(Logging::VERBOSE1)
#define LOG_LOGLEVEL_VERBOSE VLOG(Logging::VERBOSE2)

#define LOG_LOGLEVEL_FATAL_MOD(module) \
LOG_IF(FATAL, module & Logging::log_module())
#define LOG_LOGLEVEL_ERROR_MOD(module) \
LOG_IF(ERROR, module & Logging::log_module())
#define LOG_LOGLEVEL_WARNING_MOD(module) \
LOG_IF(WARNING, module & Logging::log_module())
#define LOG_LOGLEVEL_INFO_MOD(module) \
VLOG_IF(Logging::VERBOSE0, module & Logging::log_module())
#define LOG_LOGLEVEL_DEBUG_MOD(module) \
VLOG_IF(Logging::VERBOSE1, module & Logging::log_module())
#define LOG_LOGLEVEL_VERBOSE_MOD(module) \
VLOG_IF(Logging::VERBOSE2, module & Logging::log_module())

#define BTCLOG(level) LOG_##level
#define BTCLOG_MOD(level, module) LOG_##level##_MOD(module)


class Logging {
public:
	enum Module : uint16_t {
		NONE        = 0,
		NET         = (1 <<  0),
		MEMPOOL     = (1 <<  1),
		HTTP        = (1 <<  2),
		DB          = (1 <<  3),
		RPC         = (1 <<  4),
		PRUNE       = (1 <<  5),
		LIBEVENT    = (1 <<  6),
		COINDB      = (1 <<  7),
		ALL         = UINT16_MAX,
	};
	
	enum Gloglevel : uint8_t {
		FATAL = 3,
		ERROR = 2,
		WARNING = 1,
		VERBOSE0 = 4,
		VERBOSE1 = 5,
		VERBOSE2 = 6,
	};
	
	void Init(char *argv0);
	
	static uint32_t log_module()
	{
		return log_module_.load(std::memory_order_relaxed);
	}
	static void set_logModule(Logging::Module bit)
	{
		log_module_ |= bit;
	}
	
	static uint8_t MapIntoGloglevel(uint8_t loglevel)
	{
		assert(loglevel < LOGLEVEL_MAX);
		return map_loglevel_[loglevel];
	}
	static Logging::Module MapIntoModule(const std::string& str)
	{
		auto it = map_module_.find(str);
		if (it == map_module_.end()) {
			LOG(ERROR) << "Unsupported logging module: " << str;
			return Logging::NONE;
		}
		return it->second;
	}
	
private:
	static std::atomic<uint32_t> log_module_;
	static const std::array<uint8_t, LOGLEVEL_MAX> map_loglevel_;
	static const std::map<std::string, Module> map_module_;
};


#endif // BTCLITE_LOGGING_H