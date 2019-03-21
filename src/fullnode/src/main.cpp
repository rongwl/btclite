#include "clientversion.h"
#include "executor.h"
#include "sync.h"
#include "util.h"
#include "utiltime.h"
#include "hash.h"
//#include "block.h"


int main(int argc, char **argv)
{
	// Init Google's logging library
	google::InitGoogleLogging(argv[0]);
	FLAGS_logtostderr = 1;
	FLAGS_v = g_map_loglevel[std::stoi(DEFAULT_LOG_LEVEL)];
	BTCLOG(LOGLEVEL_INFO) << "default log level: " << DEFAULT_LOG_LEVEL;

	FullNodeMain fullnode(argc, argv);
	fullnode.Init();

	//Block genesis = CreateGenesisBlock(1231006505, 2083236893, 0x1d00ffff, 1, 50*satoshi_per_bitcoin);
	//std::cout << genesis.ToString() << std::endl;
	fullnode.WaitForSignal();
	fullnode.Interrupt();
	fullnode.Stop();
	
	return EXIT_SUCCESS;
}
