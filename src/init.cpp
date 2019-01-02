#include "init.h"
#include "net.h"
#include "util.h"

#include <signal.h>

std::atomic<bool> g_shutdown_requested(false);

const std::map<std::string, LogFlags::Flag> g_map_category = {
	{"0",        LogFlags::NONE},
	{"net",      LogFlags::NET},
	{"mempool",  LogFlags::MEMPOOL},
	{"http",     LogFlags::HTTP},
	{"db",       LogFlags::DB},
	{"rpc",      LogFlags::RPC},
	{"prune",    LogFlags::PRUNE},
	{"libevent", LogFlags::LIBEVENT},
	{"coindb",   LogFlags::COINDB},
	{"1",        LogFlags::ALL}
};

void Interrupt()
{
	
}

void Shutdown()
{
	LogPrint(LogLevel::NOTICE, "%s: In progress...\n", __func__);
	
	LogPrint(LogLevel::NOTICE, "%s: done\n", __func__);
}

static void new_handle_terminate()
{
	// Rather than throwing std::bad-alloc if allocation fails, terminate
    // immediately to (try to) avoid chain corruption.
    // Since LogPrintf may itself allocate memory, set the handler directly
    // to terminate first.
	std::set_new_handler(std::terminate);
	LogPrint(LogLevel::CRIT, "Critical error: out of memory. Terminating.\n");
	
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
	fprintf(stdout, "  --debug=<category>    output debugging information(default: 0)\n");
	fprintf(stdout, "                        <category> can be 1(output all debugging information),\n");
	fprintf(stdout, "                        mempool, net\n");
	fprintf(stdout, "  --conf=<file>         specify configuration file (default: %s)\n", DEFAULT_CONFIG_FILE);
	fprintf(stdout, "  --loglevel=<level>    specify the type of message being printed(default: %s)\n", DEFAULT_LOG_LEVEL);
	fprintf(stdout, "                        <level> can be:\n");
	fprintf(stdout, "                            0(A critical condition),\n");
	fprintf(stdout, "                            1(An error has occurred),\n");
	fprintf(stdout, "                            2(A warning),\n");
	fprintf(stdout, "                            3(Normal message to take note of),\n");
	fprintf(stdout, "                            4(Some information),\n");
	fprintf(stdout, "                            5(Debug information related to the program)\n");
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
		LogPrint(LogLevel::ERROR, "Error: Specified data directory \"%s\" does not exist.\n", path.c_str());
		return false;
	}
	
	g_args.ReadConfigFile(g_args.GetArg(BTCLITED_OPTION_CONF, DEFAULT_CONFIG_FILE));
	
	return true;
}

bool AppInitParameterInteraction()
{
	// --connect
	if (g_args.IsArgSet(BTCLITED_OPTION_CONNECT)) {
		g_args.SetArg(BTCLITED_OPTION_LISTEN, "0");
		LogPrint(LogLevel::NOTICE, "set --connect=1 -> set --listen=0\n");
		g_args.SetArg(BTCLITED_OPTION_DNSSEED, "0");
		LogPrint(LogLevel::NOTICE, "set --connect=1 -> set --dnsseed=0\n");
	}
	
	// --listen
	if (g_args.GetArg(BTCLITED_OPTION_LISTEN, DEFAULT_LISTEN) == "0") {
		g_args.SetArg(BTCLITED_OPTION_DISCOVER, "0");
		LogPrint(LogLevel::NOTICE, "set --listen=0 -> set --discover=0\n");
	}
	
	// --debug
	if (g_args.IsArgSet(BTCLITED_OPTION_DEBUG)) {
		const std::vector<std::string> arg_values = g_args.GetArgs(BTCLITED_OPTION_DEBUG);
		if (std::none_of(arg_values.begin(), arg_values.end(),
		[](const std::string& val) { return val == "0"; })) {
			for (auto category : arg_values) {
				auto it = g_map_category.find(category);
				if (it == g_map_category.end()) {
					LogPrint(LogLevel::ERROR, "Unsupported logging category: %s\n", category.c_str());
					return false;
				}
				g_log_categories |= it->second;
			}
		}
	}
	
	// --loglevel
	if (g_args.IsArgSet(BTCLITED_OPTION_LOGLEVEL)) {
		const std::string arg_val = g_args.GetArg(BTCLITED_OPTION_LOGLEVEL, DEFAULT_LOG_LEVEL);
		if ( arg_val.length() != 1 && std::atoi(arg_val.c_str()) > LogLevel::DEBUG) {
			LogPrint(LogLevel::ERROR, "Unsupported log level: %s\n", arg_val.c_str());
			return false;
		}
		g_log_level = std::atoi(arg_val.c_str());
		LogPrint(LogLevel::DEBUG, "set log level: %u\n", g_log_level.load(std::memory_order_relaxed));
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
