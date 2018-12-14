#include "init.h"
#include "util.h"

#include <signal.h>

std::atomic<bool> shutdown_requested(false);

void Interrupt(boost::thread_group* thread_group)
{
	if (thread_group) {
		thread_group->interrupt_all();
	}
}

void Shutdown()
{
	LogPrintf("%s: In progress...\n", __func__);
	
	LogPrintf("%s: done\n", __func__);
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
	fprintf(stdout, "Usage: btcdemod [OPTIONS...]\n\n");
	fprintf(stdout, "OPTIONS:\n");
	fprintf(stdout, "  -h or -?,  --help     Print this help message and exit\n");
	fprintf(stdout, "  --debug=<category>    Output debugging information(default: 0).\n");
	fprintf(stdout, "                        <category> can be 1(output all debugging information),\n");
	fprintf(stdout, "                        mempool, net.\n");
                  //"                                                                                "
	
	exit(EXIT_FAILURE);
}

bool AppInitParameterInteraction()
{
	output_debug = map_mutil_args.count("debug");

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
