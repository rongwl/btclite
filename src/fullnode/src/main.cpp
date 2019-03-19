#include "clientversion.h"
#include "executor.h"
#include "sync.h"
#include "util.h"
#include "utiltime.h"
#include "hash.h"
//#include "block.h"




bool AppInit(int argc, char **argv)
{
	bool ret = false;
/*
	try {
		if (!AppInitParameter(argc, argv))
			return false;
		if (!AppInitLogging(argc, argv))
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

	//Block genesis = CreateGenesisBlock(1231006505, 2083236893, 0x1d00ffff, 1, 50*satoshi_per_bitcoin);
	//std::cout << genesis.ToString() << std::endl;

	if (!ret) {
		Interrupt();
	}
	else {
		WaitForShutdown();
	}
	Shutdown();
*/
	return ret;
}

int main(int argc, char **argv)
{
	FullNodeArgs args;
	if (!args.Parse(argc, argv)) 
		return EXIT_FAILURE;

	if (!InitLogging(argc, argv, args))
		return EXIT_FAILURE;
	
	std::string path;
	if (args.IsArgSet(GLOBAL_OPTION_DATADIR)) {
		path = args.GetArg(GLOBAL_OPTION_DATADIR, "");		
	}
	FullNodeDataFiles data_files(path);

	FullNodeMain fullnode(args, data_files);
	fullnode.Init();
	fullnode.WaitForSignal();
	fullnode.Interrupt();
	fullnode.Stop();
	
	//return (AppInit(argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE);
	return EXIT_SUCCESS;
}
