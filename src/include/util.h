#ifndef BTCLITE_UTIL_H
#define BTCLITE_UTIL_H

#include "sync.h"
#include "tinyformat.h"

#include <atomic>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <map>
#include <set>
#include <thread>
#include <vector>

#if __GNUC__ >= 8
#include <filesystem>
namespace fs = std::filesystem;
#else 
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif


#define BTCLITED_OPTION_HELP     "help"
#define BTCLITED_OPTION_DATADIR  "datadir"
#define BTCLITED_OPTION_DEBUG    "debug"
#define BTCLITED_OPTION_CONF     "conf"
#define BTCLITED_OPTION_LOGLEVEL "loglevel"
#define BTCLITED_OPTION_CONNECT  "connect"
#define BTCLITED_OPTION_LISTEN   "listen"
#define BTCLITED_OPTION_DISCOVER "discover"
#define BTCLITED_OPTION_DNSSEED  "dnsseed"

#define DEFAULT_CONFIG_FILE     "btclite.conf"
#define DEFAULT_LOG_LEVEL       "3"

extern bool output_debug;
extern std::atomic<uint32_t> g_log_categories;
extern std::atomic<uint8_t> g_log_level;

void SetupEnvironment();

namespace LogFlags {
enum Flag : uint32_t {
	NONE        = 0,
	NET         = (1 <<  0),
	MEMPOOL     = (1 <<  1),
	HTTP        = (1 <<  2),
	DB          = (1 <<  3),
	RPC         = (1 <<  4),
	PRUNE       = (1 <<  5),
	LIBEVENT    = (1 <<  6),
	COINDB      = (1 <<  7),
	ALL         = ~(uint32_t)0,
};
}

namespace LogLevel {
enum Flag : uint8_t {
	CRIT,    // A critical condition
	ERROR,   // An error has occurred
	WARNING, // A warning
	NOTICE,  // Normal message to take note of
	INFO,    // Some information
	DEBUG,   // Debug information related to the program
};
}

/** Return true if log accepts specified category */
static inline bool LogAcceptCategory(uint32_t category)
{
    return (g_log_categories.load(std::memory_order_relaxed) & category) != 0;
}

static inline bool IsNeedPrint(uint8_t level)
{
	return (level <= g_log_level.load(std::memory_order_relaxed));
}

/** Send a string to the log output */
int LogPrintStr(const std::string &str);

#define LogPrint(level, ...) do { \
    if (IsNeedPrint((level))) { \
	LogPrintStr(tfm::format(__VA_ARGS__)); \
} \
} while(0)

#define LogPrintf(...) do { \
LogPrintStr(tfm::format(__VA_ARGS__)); \
} while(0)

class ArgsManager {
public:
	bool ParseParameters(int, char* const*);
	void ReadConfigFile(const std::string&) const;
	std::string GetArg(const std::string&, const std::string&) const;
	std::vector<std::string> GetArgs(const std::string&) const;
	void SetArg(const std::string&, const std::string&);
	bool IsArgSet(const std::string&) const;
private:
	mutable CCriticalSection cs_args_;
	std::map<std::string, std::string> map_args_;
	std::map<std::string, std::vector<std::string>> map_multi_args_;
	
	bool CheckOptions(int, char* const*);
};
extern ArgsManager g_args;

class PathManager {
public:
	PathManager()
		: path_(GetDefaultDataDir()) {}
	fs::path GetDataDir() const;
	void UpdateDataDir();
	fs::path GetDefaultDataDir() const;
	fs::path GetConfigFile(const std::string&) const;
private:
	mutable CCriticalSection cs_path_;
	fs::path path_;
};
extern PathManager g_path;

#endif // BTCLITE_UTIL_H
