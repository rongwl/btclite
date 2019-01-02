#include "clientversion.h"
#include "init.h"
#include "sync.h"
#include "util.h"
#include "utiltime.h"
#include <iostream>

ArgsManager g_args;
PathManager g_path;


void WaitForShutdown()
{
	bool shutdown = get_g_shutdown_requested();
	
	while (!shutdown) {
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		shutdown = get_g_shutdown_requested();
	}
	Interrupt();
}

bool AppInit(int argc, char **argv)
{
	bool ret = false;

	try {
		if (!AppInitParameter(argc, argv))
			return false;
		if (!AppInitBasicSetup())
			return false;
		if (!AppInitParameterInteraction())
			return false;
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
	SetupEnvironment();
	return (AppInit(argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE);
}
