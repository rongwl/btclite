#include "init.h"

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
	std::cout << "shutdown" << std::endl;
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
