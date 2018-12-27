#include "init.h"
#include "util.h"

#include <signal.h>

std::atomic<bool> shutdown_requested(false);

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
	
	// The log was successful, terminate now.
	std::terminate();
}

bool get_shutdown_requested()
{
	return shutdown_requested;
}
void HandleSIGTERM(int)
{
	shutdown_requested = true;
}

void PrintUsage()
{
	fprintf(stdout, "Usage: btclited [OPTIONS...]\n\n");
	fprintf(stdout, "OPTIONS:\n");
	fprintf(stdout, "  -h or -?,  --help     Print this help message and exit.\n");
	fprintf(stdout, "  --datadir=<dir>       Specify data directory.\n");
	fprintf(stdout, "  --debug=<category>    Output debugging information(default: 0).\n");
	fprintf(stdout, "                        <category> can be 1(output all debugging information),\n");
	fprintf(stdout, "                        mempool, net.\n");
	fprintf(stdout, "  --conf=<file>         Specify configuration file (default: %s)\n", BTCLITE_CONFIG_FILE);
	fprintf(stdout, "  --loglevel=<level>    Specify the type of message being printed(default: 3).\n");
	fprintf(stdout, "                        <level> can be:\n");
	fprintf(stdout, "                            0(A critical condition),\n");
	fprintf(stdout, "                            1(An error has occurred),\n");
	fprintf(stdout, "                            2(A warning),\n");
	fprintf(stdout, "                            3(Normal message to take note of),\n");
	fprintf(stdout, "                            4(Some information),\n");
	fprintf(stdout, "                            5(Debug information related to the program).\n");
                  //"                                                                                "
	
	exit(EXIT_FAILURE);
}

bool AppInitParameterInteraction()
{
	// ********************************************************* Step 3: parameter-to-internal-flags
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
	if (g_args.IsArgSet(BTCLITED_OPTION_LOGLEVEL)) {
		const std::string arg_val = g_args.GetArg(BTCLITED_OPTION_LOGLEVEL, "3");
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
	return !shutdown_requested;
}
