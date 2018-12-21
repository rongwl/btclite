#include "clientversion.h"
#include "init.h"
#include "sync.h"
#include "util.h"
#include "utiltime.h"
#include <iostream>

ArgsManger g_args;

void WaitForShutdown(boost::thread_group* thread_group)
{
	bool shutdown = get_shutdown_requested();
	
	while (!shutdown) {
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		shutdown = get_shutdown_requested();
	}
	if (thread_group) {
		thread_group->join_all();
	}
}

bool AppInit(int argc, char **argv)
{
	boost::thread_group thread_group;
	bool ret = false;
	g_args.ParseParameters(argc, argv);

	try {
		if (!AppInitBasicSetup())
			exit(EXIT_FAILURE);
		if (!AppInitParameterInteraction())
			exit(EXIT_FAILURE);
		ret = AppInitMain();
	}
	catch (std::exception& e) {
	
	}
	catch (...) {
	
	}
	
	if (!ret) {
		Interrupt(&thread_group);
	}
	else {
		WaitForShutdown(&thread_group);
	}
	Shutdown();

	return ret;
}

int main(int argc, char **argv)
{
	return (AppInit(argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE);
}
