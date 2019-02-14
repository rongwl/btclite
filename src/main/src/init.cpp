#include "init.h"
#include "net.h"
#include "util.h"

#include <signal.h>

std::atomic<bool> g_shutdown_requested(false);

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

void Interrupt()
{
	
}

void Shutdown()
{
	BTCLOG(LOGLEVEL_INFO) << __func__ << ": progress...";
	
	BTCLOG(LOGLEVEL_INFO) << __func__ << ": done";
}

static void new_handle_terminate()
{
	// Rather than throwing std::bad-alloc if allocation fails, terminate
    // immediately to (try to) avoid chain corruption.
	std::set_new_handler(std::terminate);
	BTCLOG(LOGLEVEL_ERROR) << "Critical error: out of memory. Terminating.";
	
	// The log was successful, terminate now.
	std::terminate();
}

bool get_g_shutdown_requested()
{
	return g_shutdown_requested;
}
void HandleSIGTERM(int)
{
	g_shutdown_requested = true;
}

void PrintUsage()
{
	fprintf(stdout, "Usage: btclited [OPTIONS...]\n\n");
	fprintf(stdout, "Common Options:\n");
	fprintf(stdout, "  -h or -?,  --help     print this help message and exit\n");
	fprintf(stdout, "  --datadir=<dir>       specify data directory.\n");
	fprintf(stdout, "  --debug=<module>      output debugging information(default: 0)\n");
	fprintf(stdout, "                        <module> can be 1(output all debugging information),\n");
	fprintf(stdout, "                        mempool, net\n");
	fprintf(stdout, "  --conf=<file>         specify configuration file (default: %s)\n", DEFAULT_CONFIG_FILE);
	fprintf(stdout, "  --loglevel=<level>    specify the type of message being printed(default: %s)\n", DEFAULT_LOG_LEVEL);
	fprintf(stdout, "                        <level> can be:\n");
	fprintf(stdout, "                            0(A fatal condition),\n");
	fprintf(stdout, "                            1(An error has occurred),\n");
	fprintf(stdout, "                            2(A warning),\n");
	fprintf(stdout, "                            3(Normal message),\n");
	fprintf(stdout, "                            4(Debug information),\n");
	fprintf(stdout, "                            5(Verbose information\n");
	fprintf(stdout, "\nConnection Options:\n");
	fprintf(stdout, "  --connect=<ip>        connect only to the specified node(s); -connect=0 \n");
	fprintf(stdout, "                        disables automatic connections\n");
	fprintf(stdout, "  --listen=<1/0>        accept connections from outside (default: %s)\n", DEFAULT_LISTEN);
	fprintf(stdout, "  --discover=<1/0>      discover own ip address (default: %s)\n", DEFAULT_DISCOVER);
	fprintf(stdout, "  --dnsseed=<1/0>       query for peer addresses via DNS lookup (default: %s)\n", DEFAULT_DNSSEED);
                  //"                                                                                "
	
	exit(EXIT_FAILURE);
}

bool AppInitParameter(int argc, char* const argv[])
{
	if (!g_args.ParseParameters(argc, argv))
		return false;

	g_path.UpdateDataDir();
	fs::path path = g_path.GetDataDir();
	fs::create_directories(path);
	if (!fs::is_directory(path)) {
		BTCLOG(LOGLEVEL_ERROR) << "Error: Specified data directory \"" << path.c_str() << "\" does not exist.";
		return false;
	}
	
	g_args.ReadConfigFile(g_args.GetArg(BTCLITED_OPTION_CONF, DEFAULT_CONFIG_FILE));
	
	return true;
}

bool AppInitLogging(int argc, char* const argv[])
{
	// Init Google's logging library
	google::InitGoogleLogging(argv[0]);
	FLAGS_logtostderr = 1;
	FLAGS_v = g_map_loglevel[std::stoi(DEFAULT_LOG_LEVEL)];
	
	return true;
}

bool AppInitParameterInteraction()
{
	// --connect
	if (g_args.IsArgSet(BTCLITED_OPTION_CONNECT)) {
		g_args.SetArg(BTCLITED_OPTION_LISTEN, "0");
		BTCLOG(LOGLEVEL_DEBUG) << "set --connect=1 -> set --listen=0";
		g_args.SetArg(BTCLITED_OPTION_DNSSEED, "0");
		BTCLOG(LOGLEVEL_DEBUG) << "set --connect=1 -> set --dnsseed=0";
	}
	
	// --listen
	if (g_args.GetArg(BTCLITED_OPTION_LISTEN, DEFAULT_LISTEN) == "0") {
		g_args.SetArg(BTCLITED_OPTION_DISCOVER, "0");
		BTCLOG(LOGLEVEL_DEBUG) << "set --listen=0 -> set --discover=0";
	}
	
	// --debug
	if (g_args.IsArgSet(BTCLITED_OPTION_DEBUG)) {
		const std::vector<std::string> arg_values = g_args.GetArgs(BTCLITED_OPTION_DEBUG);
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
	
	// --loglevel
	if (g_args.IsArgSet(BTCLITED_OPTION_LOGLEVEL)) {
		const std::string arg_val = g_args.GetArg(BTCLITED_OPTION_LOGLEVEL, DEFAULT_LOG_LEVEL);
		if ( arg_val.length() != 1 && std::stoi(arg_val.c_str()) >= LOGLEVEL_MAX) {
			BTCLOG(LOGLEVEL_ERROR) << "Unsupported log level: " << arg_val.c_str();
			return false;
		}
		//g_log_level = std::atoi(arg_val.c_str());
		uint8_t level = std::stoi(arg_val.c_str());
		if (level <= LOGLEVEL_WARNING)
			FLAGS_minloglevel = g_map_loglevel[level];
		else {
			FLAGS_minloglevel = 0;
			FLAGS_v = g_map_loglevel[level];
		}
		BTCLOG(LOGLEVEL_VERBOSE) << "set log level: " << level;
	}

	return true;
}

bool AppInitBasicSetup()
{	
	// Clean shutdown on SIGTERM
	struct sigaction sa;
	sa.sa_handler = HandleSIGTERM;
	sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	
	// Ignore SIGPIPE, otherwise it will bring the daemon down if the client closes unexpectedly
    signal(SIGPIPE, SIG_IGN);
	
	std::set_new_handler(new_handle_terminate);
	
	return true;
}

bool AppInitMain()
{
	return !g_shutdown_requested;
}
