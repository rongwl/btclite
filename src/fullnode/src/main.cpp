#include "error.h"
#include "executor.h"
//#include "utility/include/logging.h"
//#include "block.h"


int main(int argc, char **argv)
{
    btclite::util::logging::InitLogging(argv[0]);
    
    btclite::fullnode::FullNodeConfig config;
    try {
        config.ParseParameters(argc, argv);
    }
    catch (const btclite::util::Exception& e) {
        if (e.code().value() != 
                static_cast<std::underlying_type_t<btclite::util::ErrorCode> >(btclite::util::ErrorCode::kShowHelp))
            fprintf(stderr, "%s: %s\n", argv[0], e.what());
        config.PrintUsage(FULLNODE_BIN_NAME);
        exit(e.code().value());
    }
    
    if (!config.InitArgs())
        return EXIT_FAILURE;
    
    if (!config.InitDataDir())
        return EXIT_FAILURE;
    
    btclite::fullnode::FullNode fullnode(config);
    bool ret = false;
    try {
        ret = fullnode.Init();
        if (ret)
            ret = fullnode.Start();
    }
    catch (const btclite::util::Exception& e) {
        fprintf(stderr, "Error: %s\n", e.what());
    }
    catch (const std::exception& e) {
        fprintf(stderr, "Error: %s\n", e.what());
    }
    catch (...) {
        
    }

    if (ret) {
        fullnode.WaitToStop();
    }
    
    fullnode.Interrupt();
    fullnode.Stop();
    
    //Block genesis = CreateGenesisBlock(1231006505, 2083236893, 0x1d00ffff, 1, 50*kSatoshiPerBitcoin);
    //std::cout << genesis.ToString() << std::endl;
    
    return EXIT_SUCCESS;
}
