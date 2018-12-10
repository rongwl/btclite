#include "clientversion.h"
#include "init.h"
#include "util.h"
#include "utiltime.h"
#include <iostream>

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
	
	ParseParameters(argc, argv);
	if (IsArgSet("-?") || IsArgSet("-h") || IsArgSet("-help") || IsArgSet("-version")) {
		std::string usage = std::string(PACKAGE_STRING) + std::string("\n");
		if (IsArgSet("-version")) {
		
		}
		else {
			usage += std::string("\nUsage:\n  btcdemod [options]                     Start ") +
					 std::string(PACKAGE_NAME) + std::string(" Daemon\n\n");
			usage += HelpMessage();
		}
		std::cout << usage;
		return true;
	}
	
	try {
		if (!AppInitBasicSetup())
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
