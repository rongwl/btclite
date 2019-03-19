#if defined(HAVE_CONFIG_H)
#include "config/btclite-config.h"
#endif

#include "serialize.h"
#include "sync.h"
#include "util.h"
#include "utiltime.h"

#include <getopt.h>
#include <locale>
#include <stdarg.h>

bool output_debug = false;
bool print_to_console = true;
bool print_to_file = false;

std::atomic<uint32_t> g_log_module(0);
std::atomic<uint8_t> g_log_level(LOGLEVEL_INFO); 
volatile std::sig_atomic_t Signal::received_signal_ = 0;

const std::map<std::string, LogModule::Flag> g_map_module = {
	{"0",        LogModule::NONE},
	{"net",      LogModule::NET},
	{"mempool",  LogModule::MEMPOOL},
	{"http",     LogModule::HTTP},
	{"db",       LogModule::DB},
	{"rpc",      LogModule::RPC},
	{"prune",    LogModule::PRUNE},
	{"libevent", LogModule::LIBEVENT},
	{"coindb",   LogModule::COINDB},
	{"1",        LogModule::ALL}
};
const std::array<uint8_t, LOGLEVEL_MAX> g_map_loglevel = {
	GlogLevel::FATAL,    // LOGLEVEL_FATAL map into GlogLevel::FATAL
	GlogLevel::ERROR,    // LOGLEVEL_ERROR map into GlogLevel::ERROR
	GlogLevel::WARNING,  // LOGLEVEL_WARNING map into GlogLevel::WARNING
	GlogLevel::VERBOSE0, // LOGLEVEL_INFO map into GlogLevel::VERBOSE0
	GlogLevel::VERBOSE1, // LOGLEVEL_DEBUG map into GlogLevel::VERBOSE1
	GlogLevel::VERBOSE2  // LOGLEVEL_VERBOSE map into GlogLevel::VERBOSE2
};

/**
 * started_newline is a state variable held by the calling context that will
 * suppress printing of the timestamp when multiple calls are made that don't
 * end in a newline. Initialize it to true, and hold it, in the calling context.
 */
static std::string LogTimestampStr(const std::string &str, std::atomic_bool *started_newline)
{
    std::string str_stamped;

    if (*started_newline)
        str_stamped = DateTimeStrFormat() + " " + str;
    else
        str_stamped = str;

    if (!str.empty() && str[str.size()-1] == '\n')
        *started_newline = true;
    else
        *started_newline = false;

    return str_stamped;
}

int LogPrintStr(const std::string &str)
{
    int ret = 0; // Returns total number of characters written
    static std::atomic_bool started_newline(true);

    std::string str_time_stamped = LogTimestampStr(str, &started_newline);

    ret = std::fwrite(str_time_stamped.data(), 1, str_time_stamped.size(), stdout);
    fflush(stdout);
    
    return ret;
}

static void HandleAllocFail()
{
	// Rather than throwing std::bad-alloc if allocation fails, terminate
    // immediately to (try to) avoid chain corruption.
	std::set_new_handler(std::terminate);
	BTCLOG(LOGLEVEL_ERROR) << "Critical error: out of memory. Terminating.";
	
	// The log was successful, terminate now.
	std::terminate();
}

bool InitLogging(int argc, char* const argv[], const ArgsManager& args)
{
	// Init Google's logging library
	google::InitGoogleLogging(argv[0]);
	FLAGS_logtostderr = 1;
	
	// --loglevel
	if (args.IsArgSet(GLOBAL_OPTION_LOGLEVEL)) {
		const std::string arg_val = args.GetArg(GLOBAL_OPTION_LOGLEVEL, DEFAULT_LOG_LEVEL);
		if ( arg_val.length() != 1 && std::stoi(arg_val.c_str()) >= LOGLEVEL_MAX) {
			BTCLOG(LOGLEVEL_ERROR) << "Unsupported log level: " << arg_val.c_str();
			return false;
		}

		uint8_t level = std::stoi(arg_val.c_str());
		if (level <= LOGLEVEL_WARNING)
			FLAGS_minloglevel = g_map_loglevel[level];
		else {
			FLAGS_minloglevel = 0;
			FLAGS_v = g_map_loglevel[level];
		}
		BTCLOG(LOGLEVEL_VERBOSE) << "set log level: " << +level;
	}
	else {
		FLAGS_v = g_map_loglevel[std::stoi(DEFAULT_LOG_LEVEL)];
		BTCLOG(LOGLEVEL_INFO) << "default log level: " << DEFAULT_LOG_LEVEL;
	}
	
	return true;
}

void ArgsManager::PrintUsage()
{
	fprintf(stdout, "Usage: btclited [OPTIONS...]\n\n");
	fprintf(stdout, "Common Options:\n");
	fprintf(stdout, "  -h or -?,  --help     print this help message and exit\n");
	fprintf(stdout, "  --debug=<module>      output debugging information(default: 0)\n");
	fprintf(stdout, "                        <module> can be 1(output all debugging information),\n");
	fprintf(stdout, "                        mempool, net\n");
	fprintf(stdout, "  --loglevel=<level>    specify the type of message being printed(default: %s)\n", DEFAULT_LOG_LEVEL);
	fprintf(stdout, "                        <level> can be:\n");
	fprintf(stdout, "                            0(A fatal condition),\n");
	fprintf(stdout, "                            1(An error has occurred),\n");
	fprintf(stdout, "                            2(A warning),\n");
	fprintf(stdout, "                            3(Normal message),\n");
	fprintf(stdout, "                            4(Debug information),\n");
	fprintf(stdout, "                            5(Verbose information\n");
	//              "                                                                                "

}

/* Check options that getopt_long() can not print totally */
bool ArgsManager::CheckOptions(int argc, char* const argv[])
{
	for (int i = 1; i < argc; i++) {
		std::string str(argv[i]);
		if ((str.length() > 2 && str.compare(0, 2, "--")) ||
		    !str.compare("--")) {
			fprintf(stdout, "%s: invalid option '%s'\n", argv[0], str.c_str());
			return false;
		}
	}
	return true;
}

std::string ArgsManager::GetArg(const std::string& arg, const std::string& arg_default) const
{
	LOCK(cs_args_);
	auto it = map_args_.find(arg);
	if (it != map_args_.end())
		return it->second;
	return arg_default;
}

std::vector<std::string> ArgsManager::GetArgs(const std::string& arg) const
{
	LOCK(cs_args_);
	auto it = map_multi_args_.find(arg);
	if (it != map_multi_args_.end())
		return it->second;
	return {};
}

void ArgsManager::SetArg(const std::string& arg, const std::string& arg_val)
{
	LOCK(cs_args_);
	map_args_[arg] = arg_val;
	map_multi_args_[arg] = {arg_val};
}

bool ArgsManager::IsArgSet(const std::string& arg) const
{
	LOCK(cs_args_);
	return map_args_.count(arg);
}

void ArgsManager::ParseFromFile(const std::string& path) const
{
	std::ifstream ifs(path);
	if (!ifs.good()) {
		return; // No config file is OK
	}
	 
	std::string line;
	while (std::getline(ifs, line)) {
	 line.erase(std::remove_if(line.begin(), line.end(), 
			[](unsigned char x){return std::isspace(x);}),
		 line.end());
	 if (line[0] == '#' || line.empty())
	  continue;
	 auto pos = line.find("=");
	 if (pos != std::string::npos) {
	  std::string str = line.substr(0, pos);
	  if (!IsArgSet(str) && str != GLOBAL_OPTION_CONF) {
	   // Don't overwrite existing settings so command line settings override config file
	   std::string str_val = line.substr(pos+1);
	  }
	 }
	}
}

bool BaseExecutor::BasicSetup()
{	
	// Ignore SIGPIPE, otherwise it will bring the daemon down if the client closes unexpectedly
    std::signal(SIGPIPE, SIG_IGN);
	
	std::set_new_handler(HandleAllocFail);
	
	return true;
}

void BaseExecutor::WaitForSignal()
{
	while (!sig_int_.IsReceived() && !sig_term_.IsReceived()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
}

bool BaseExecutor::InitParameters()
{
	// --debug
	if (args_.IsArgSet(GLOBAL_OPTION_DEBUG)) {
		const std::vector<std::string> arg_values = args_.GetArgs(GLOBAL_OPTION_DEBUG);
		if (std::none_of(arg_values.begin(), arg_values.end(),
		[](const std::string& val) { return val == "0"; })) {
			for (auto module : arg_values) {
				auto it = g_map_module.find(module);
				if (it == g_map_module.end()) {
					BTCLOG(LOGLEVEL_ERROR) << "Unsupported logging module: " << module.c_str();
					return false;
				}
				g_log_module |= it->second;
			}
		}
	}
	
	return true;
}

void SetupEnvironment()
{
	try {
        std::locale(""); // Raises a runtime error if current locale is invalid
    } catch (const std::runtime_error&) {
        setenv("LC_ALL", "C", 1);
    }
}

