#ifndef BTCLITE_UTIL_H
#define BTCLITE_UTIL_H

#include "sync.h"
#include "tinyformat.h"

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <getopt.h>
#include <map>
#include <set>
#include <thread>
#include <vector>

#include <glog/logging.h>

#if __GNUC__ >= 8
#include <filesystem>
namespace fs = std::filesystem;
#else 
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#define GLOBAL_OPTION_HELP     "help"
#define GLOBAL_OPTION_DATADIR  "datadir"
#define GLOBAL_OPTION_DEBUG    "debug"
#define GLOBAL_OPTION_CONF     "conf"
#define GLOBAL_OPTION_LOGLEVEL "loglevel"

#define DEFAULT_LOG_LEVEL       "3"

extern bool output_debug;
extern std::atomic<uint32_t> g_log_module;
extern std::atomic<uint8_t> g_log_level;


class SigMonitor {
public:
	SigMonitor(volatile std::sig_atomic_t sig)
		: signal_(sig)
	{
	    std::signal(sig, Handler);
	}
	
	bool IsReceived() const
	{
		return (received_signal_ != 0 && received_signal_ == signal_);
	}

	static volatile std::sig_atomic_t received_signal_;

private:
	volatile std::sig_atomic_t signal_;
	
	static void Handler(int sig)
	{
		received_signal_ = sig;
	}
};

// mixin class
template <class BASE>
class Uncopyable : public BASE {
public:
	using BASE::BASE;
    Uncopyable(const Uncopyable&) = delete;
    void operator=(const Uncopyable&) = delete;
};

namespace LogModule {
enum Flag : uint16_t {
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
}

#define LOGLEVEL_FATAL   0
#define LOGLEVEL_ERROR   1
#define LOGLEVEL_WARNING 2
#define LOGLEVEL_INFO    3
#define LOGLEVEL_DEBUG   4
#define LOGLEVEL_VERBOSE 5
#define LOGLEVEL_MAX     6

namespace GlogLevel {
enum Flag : uint8_t {
	FATAL = 3,
	ERROR = 2,
	WARNING = 1,
	VERBOSE0 = 4,
	VERBOSE1 = 5,
	VERBOSE2 = 6,
};
}

extern const std::array<uint8_t, LOGLEVEL_MAX> g_map_loglevel;

/** Return true if log accepts specified category */
static inline bool LogAcceptCategory(uint32_t category)
{
    return (g_log_module.load(std::memory_order_relaxed) & category) != 0;
}

static inline bool IsNeedPrint(uint8_t level)
{
	return (level <= g_log_level.load(std::memory_order_relaxed));
}

#define LOG_LOGLEVEL_FATAL   LOG(FATAL)
#define LOG_LOGLEVEL_ERROR   LOG(ERROR)
#define LOG_LOGLEVEL_WARNING LOG(WARNING)
#define LOG_LOGLEVEL_INFO    VLOG(g_map_loglevel[LOGLEVEL_INFO])
#define LOG_LOGLEVEL_DEBUG   VLOG(g_map_loglevel[LOGLEVEL_DEBUG])
#define LOG_LOGLEVEL_VERBOSE VLOG(g_map_loglevel[LOGLEVEL_VERBOSE])

#define LOG_LOGLEVEL_FATAL_MOD(module) \
	LOG_IF(FATAL, module & g_log_module.load(std::memory_order_relaxed))
#define LOG_LOGLEVEL_ERROR_MOD(module) \
	LOG_IF(ERROR, module & g_log_module.load(std::memory_order_relaxed))
#define LOG_LOGLEVEL_WARNING_MOD(module) \
	LOG_IF(WARNING, module & g_log_module.load(std::memory_order_relaxed))
#define LOG_LOGLEVEL_INFO_MOD(module) \
	VLOG_IF(g_map_loglevel[LOGLEVEL_INFO], module & g_log_module.load(std::memory_order_relaxed))
#define LOG_LOGLEVEL_DEBUG_MOD(module) \
	VLOG_IF(g_map_loglevel[LOGLEVEL_DEBUG], module & g_log_module.load(std::memory_order_relaxed))
#define LOG_LOGLEVEL_VERBOSE_MOD(module) \
	VLOG_IF(g_map_loglevel[LOGLEVEL_VERBOSE], module & g_log_module.load(std::memory_order_relaxed))


#define BTCLOG(level) LOG_##level
#define BTCLOG_MOD(level, module) LOG_##level##_MOD(module)

/** Send a string to the log output */
int LogPrintStr(const std::string &str);
/*
#define LogPrint(level, ...) do { \
    if (IsNeedPrint((level))) { \
	LogPrintStr(tfm::format(__VA_ARGS__)); \
} \
} while(0)

#define LogPrintf(...) do { \
LogPrintStr(tfm::format(__VA_ARGS__)); \
} while(0)
*/

class ArgsManager {
public:	
	virtual bool Init(int argc, char * const argv[]) = 0;
	void ParseFromFile(const std::string& path) const;
	
	std::string GetArg(const std::string& arg, const std::string& arg_default) const;
	std::vector<std::string> GetArgs(const std::string& arg) const;
	void SetArg(const std::string& arg, const std::string& arg_val);
	bool IsArgSet(const std::string& arg) const;
	
	//-------------------------------------------------------------------------
	const std::map<std::string, std::string>& MapArgs() const
	{
		LOCK(cs_args_);
		return map_args_;
	}
	const std::map<std::string, std::vector<std::string> >& MpaMultiArgs() const
	{
		LOCK(cs_args_);
		return map_multi_args_;
	}
	
protected:	
	mutable CCriticalSection cs_args_;
	std::map<std::string, std::string> map_args_;
	std::map<std::string, std::vector<std::string> > map_multi_args_;

	virtual bool Parse(int argc, char* const argv[]) = 0;
	virtual void PrintUsage();
	virtual bool InitParameters();
	bool InitLogging(const char *argv0);
	bool CheckOptions(int argc, char* const argv[]);
};

class DataFilesManager {
public:
	const fs::path& DataDir() const
	{
		LOCK(cs_path_);
		return data_dir_;
	}
	void set_dataDir(const fs::path& path)
	{
		LOCK(cs_path_);
		data_dir_ = path;
	}
	
	fs::path ConfigFile() const
	{
		LOCK(cs_path_);
		return data_dir_ / config_file_;
	}
	void set_configFile(const std::string& filename)
	{
		LOCK(cs_path_);
		config_file_ = filename;
	}
	
protected:	
	DataFilesManager(const std::string& path)
		: data_dir_(fs::path(path)) {}
	
	mutable CCriticalSection cs_path_;
	fs::path data_dir_;
	std::string config_file_;
	
	//fs::path StrToPath(const std::string& str_path) const;	
};

class BaseExecutor {
public:	
	BaseExecutor(ArgsManager& args, DataFilesManager& data_files)
		: args_(args), data_files_(data_files), sig_int_(SigMonitor(SIGINT)), sig_term_(SigMonitor(SIGTERM)) {}
	
	virtual bool Init() = 0;
	virtual bool Start() = 0;
	virtual bool Run() = 0;
	virtual void Interrupt() = 0;
	virtual void Stop() = 0;
	
	virtual bool BasicSetup();
	virtual void WaitForSignal();
	
protected:	
	ArgsManager& args_;
	DataFilesManager& data_files_;

private:
	SigMonitor sig_int_;
	SigMonitor sig_term_;
};
// mixin uncopyable
using Executor = Uncopyable<BaseExecutor>;

void SetupEnvironment();

#endif // BTCLITE_UTIL_H
