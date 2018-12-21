#include "clientversion.h"
#include "init.h"
#include "sync.h"
#include "util.h"
#include "utiltime.h"
#include <iostream>

ArgsManger g_args;

void WaitForShutdown()
{
	bool shutdown = get_shutdown_requested();
	
	while (!shutdown) {
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		shutdown = get_shutdown_requested();
	}
	Interrupt();
}

bool AppInit(int argc, char **argv)
{
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
		Interrupt();
	}
	else {
		WaitForShutdown();
	}
	Shutdown();

	return ret;
}

int main(int argc, char **argv)
{
	return (AppInit(argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE);
}
