#include "clientversion.h"
#include "init.h"
#include "sync.h"
#include "util.h"
#include "utiltime.h"
#include "hash.h"

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
	std::vector<uint8_t> v = { 'h', 'e', 'l', 'l', 'o' };
	Hash256 hash;
	DoubleSha256(v, &hash);
	std::cout << hash.Hex() << std::endl;
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
