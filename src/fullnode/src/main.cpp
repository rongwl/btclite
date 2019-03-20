#include "clientversion.h"
#include "executor.h"
#include "sync.h"
#include "util.h"
#include "utiltime.h"
#include "hash.h"
//#include "block.h"


int main(int argc, char **argv)
{
	FullNodeArgs args;
	if (!args.Init(argc, argv)) 
		return EXIT_FAILURE;
	
	std::string path;
	if (args.IsArgSet(GLOBAL_OPTION_DATADIR)) {
		path = args.GetArg(GLOBAL_OPTION_DATADIR, "");		
	}
	FullNodeDataFiles data_files(path);

	FullNodeMain fullnode(args, data_files);
	fullnode.Init();
	//Block genesis = CreateGenesisBlock(1231006505, 2083236893, 0x1d00ffff, 1, 50*satoshi_per_bitcoin);
	//std::cout << genesis.ToString() << std::endl;
	fullnode.WaitForSignal();
	fullnode.Interrupt();
	fullnode.Stop();
	
	return EXIT_SUCCESS;
}
