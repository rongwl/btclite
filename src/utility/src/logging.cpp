#include "utility/include/logging.h"


namespace btclite {
namespace util {

namespace logging {

static std::atomic<uint32_t> log_module_ = 0;

static const std::array<uint8_t, LOG_LEVEL_MAX> map_loglevel_ = {
    FATAL,    // LOG_LEVEL_FATAL map into FATAL
    ERROR,    // LOG_LEVEL_ERROR map into ERROR
    WARNING,  // LOG_LEVEL_WARNING map into WARNING
    VERBOSE0, // LOG_LEVEL_INFO map into VERBOSE0
    VERBOSE1, // LOG_LEVEL_DEBUG map into VERBOSE1
    VERBOSE2  // LOG_LEVEL_VERBOSE map into VERBOSE2
};

static const std::map<std::string, Module> map_module_ = {
    {"0",        NONE},
    {"net",      NET},
    {"mempool",  MEMPOOL},
    {"http",     HTTP},
    {"db",       DB},
    {"rpc",      RPC},
    {"prune",    PRUNE},
    {"libevent", LIBEVENT},
    {"coindb",   COINDB},
    {"1",        ALL}
};

uint32_t log_module()
{
    return log_module_.load(std::memory_order_relaxed);
}

void set_logModule(Module bit)
{
    log_module_ |= bit;
}

uint8_t MapIntoGloglevel(uint8_t loglevel)
{
    assert(loglevel < LOG_LEVEL_MAX);
    return map_loglevel_[loglevel];
}

Module MapIntoModule(const std::string& str)
{
    auto it = map_module_.find(str);
    if (it == map_module_.end()) {
        //LOG(ERROR) << "Unsupported logging module: " << str;
        return NONE;
    }
    return it->second;
}

void InitLogging(char *argv0)
{
    // Init Google's logging library
    google::InitGoogleLogging(argv0);
    FLAGS_logtostderr = 1;
    FLAGS_v = map_loglevel_[std::stoi(DEFAULT_LOG_LEVEL)];
}

} // namespace logging

} // namespace util
} // namespace btclite
