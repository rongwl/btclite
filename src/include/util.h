#ifndef BTCLITE_UTIL_H
#define BTCLITE_UTIL_H

#include "sync.h"
#include "tinyformat.h"

#include <atomic>
#include <exception>
#include <cstdlib>
#include <map>
#include <set>
#include <thread>

#include <boost/thread/thread.hpp>

#define BTCLITED_OPTION_HELP  "help"
#define BTCLITED_OPTION_DEBUG "debug"

extern bool output_debug;
extern std::atomic<uint32_t> g_log_categories;

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

/** Return true if log accepts specified category */
static inline bool LogAcceptCategory(uint32_t category)
{
    return (g_log_categories.load(std::memory_order_relaxed) & category) != 0;
}

/** Send a string to the log output */
int LogPrintStr(const std::string &str);

#define LogPrint(category, ...) do { \
    if (LogAcceptCategory((category))) { \
	LogPrintStr(tfm::format(__VA_ARGS__)); \
} \
} while(0)

#define LogPrintf(...) do { \
LogPrintStr(tfm::format(__VA_ARGS__)); \
} while(0)

class ArgsManger {
public:
	void ParseParameters(int, char* const*);
	std::vector<std::string> GetArgs(const std::string&) const;
	bool IsArgSet(const std::string&) const;
private:
	mutable CCriticalSection cs_args_;
	std::map<std::string, std::string> map_args_;
	std::map<std::string, std::vector<std::string>> map_multi_args_;
	
	void CheckOptions(int, char* const*);
};
extern ArgsManger g_args;


#endif // BTCLITE_UTIL_H
