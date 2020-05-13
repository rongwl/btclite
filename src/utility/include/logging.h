#ifndef BTCLITE_LOGGING_H
#define BTCLITE_LOGGING_H

#include <array>
#include <atomic>
#include <cassert>
#include <string>
#include <map>

#include <glog/logging.h>


namespace btclite {
namespace util {

namespace logging {

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
    ALL         = std::numeric_limits<uint16_t>::max(),
};

enum Gloglevel : uint8_t {
    FATAL = 3,
    ERROR = 2,
    WARNING = 1,
    VERBOSE0 = 4,
    VERBOSE1 = 5,
    VERBOSE2 = 6,
};

uint32_t log_module();
void set_logModule(Module bit);
uint8_t MapIntoGloglevel(uint8_t loglevel);
Module MapIntoModule(const std::string& str);
void InitLogging(char *argv0);

} // namespace logging

#define DEFAULT_LOG_LEVEL  "3"

#define LOG_LEVEL_FATAL    0
#define LOG_LEVEL_ERROR    1
#define LOG_LEVEL_WARNING  2
#define LOG_LEVEL_INFO     3
#define LOG_LEVEL_DEBUG    4
#define LOG_LEVEL_VERBOSE  5
#define LOG_LEVEL_MAX      6

#define GLOG_LOG_LEVEL_FATAL   LOG(FATAL)
#define GLOG_LOG_LEVEL_ERROR   LOG(ERROR)
#define GLOG_LOG_LEVEL_WARNING LOG(WARNING)
#define GLOG_LOG_LEVEL_INFO    VLOG(btclite::util::logging::VERBOSE0)
#define GLOG_LOG_LEVEL_DEBUG   VLOG(btclite::util::logging::VERBOSE1)
#define GLOG_LOG_LEVEL_VERBOSE VLOG(btclite::util::logging::VERBOSE2)

#define GLOG_LOG_LEVEL_FATAL_MOD(module) \
LOG_IF(FATAL, module & btclite::util::logging::log_module())
#define GLOG_LOG_LEVEL_ERROR_MOD(module) \
LOG_IF(ERROR, module & btclite::util::logging::log_module())
#define GLOG_LOG_LEVEL_WARNING_MOD(module) \
LOG_IF(WARNING, module & btclite::util::logging::log_module())
#define GLOG_LOG_LEVEL_INFO_MOD(module) \
VLOG_IF(btclite::util::logging::VERBOSE0, module & btclite::util::logging::log_module())
#define GLOG_LOG_LEVEL_DEBUG_MOD(module) \
VLOG_IF(btclite::util::logging::VERBOSE1, module & btclite::util::logging::log_module())
#define GLOG_LOG_LEVEL_VERBOSE_MOD(module) \
VLOG_IF(btclite::util::logging::VERBOSE2, module & btclite::util::logging::log_module())

#define BTCLOG(level) GLOG_##level
#define BTCLOG_MOD(level, module) GLOG_##level##_MOD(module)


} // namespace util
} // namespace btclite


#endif // BTCLITE_LOGGING_H
