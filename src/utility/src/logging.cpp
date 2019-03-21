#include "logging.h"

std::atomic<uint32_t> Logging::log_module_ = 0;

const std::array<uint8_t, LOGLEVEL_MAX> Logging::map_loglevel_ = {
	Logging::FATAL,    // LOGLEVEL_FATAL map into Logging::FATAL
	Logging::ERROR,    // LOGLEVEL_ERROR map into Logging::ERROR
	Logging::WARNING,  // LOGLEVEL_WARNING map into Logging::WARNING
	Logging::VERBOSE0, // LOGLEVEL_INFO map into Logging::VERBOSE0
	Logging::VERBOSE1, // LOGLEVEL_DEBUG map into Logging::VERBOSE1
	Logging::VERBOSE2  // LOGLEVEL_VERBOSE map into Logging::VERBOSE2
};

const std::map<std::string, Logging::Module> Logging::map_module_ = {
	{"0",        Logging::NONE},
	{"net",      Logging::NET},
	{"mempool",  Logging::MEMPOOL},
	{"http",     Logging::HTTP},
	{"db",       Logging::DB},
	{"rpc",      Logging::RPC},
	{"prune",    Logging::PRUNE},
	{"libevent", Logging::LIBEVENT},
	{"coindb",   Logging::COINDB},
	{"1",        Logging::ALL}
};

void Logging::Init(char *argv0)
{
	// Init Google's logging library
	google::InitGoogleLogging(argv0);
	FLAGS_logtostderr = 1;
	FLAGS_v = map_loglevel_[std::stoi(DEFAULT_LOG_LEVEL)];
}
