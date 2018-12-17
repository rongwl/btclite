#ifndef BTCLITE_UTIL_H
#define BTCLITE_UTIL_H

#include "tinyformat.h"

#include <atomic>
#include <exception>
#include <cstdlib>
#include <map>
#include <set>
#include <thread>

#include <boost/thread/thread.hpp>

extern const std::map<std::string, std::vector<std::string>>& map_mutil_args;
extern bool output_debug;

/** Return true if log accepts specified category */
bool LogAcceptCategory(const char* category);
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

void ParseParameters(int, char* const*);
bool IsArgSet(const std::string&);

#endif // BTCLITE_UTIL_H
