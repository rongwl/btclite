#ifndef BTCDEMO_UTIL_H
#define BTCDEMO_UTIL_H

#include "tinyformat.h"

#include <atomic>
#include <exception>
#include <cstdlib>
#include <map>
#include <set>
#include <thread>

#include <boost/thread/thread.hpp>

void ParseParameters(int, const char* const*);
bool IsArgSet(const std::string&);
std::string HelpMessageGroup(const std::string&);
std::string HelpMessageOpt(const std::string&, const std::string&);

#endif // BTCDEMO_UTIL_H
