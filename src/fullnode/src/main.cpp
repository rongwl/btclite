#include "error.h"
#include "executor.h"
//#include "utility/include/logging.h"
//#include "block.h"


int main(int argc, char **argv)
{
    Logging fullnode_logging;
    fullnode_logging.Init(argv[0]);
    
    FullNodeConfig config;
    try {
        config.ParseParameters(argc, argv);
    }
    catch (const Exception& e) {
        if (e.code().value() != ErrorCode::show_help)
            fprintf(stderr, "%s: %s\n", argv[0], e.what());
        FullNodeHelpInfo::PrintUsage();
        exit(e.code().value());
    }
    
    if (!config.InitDataDir())
        return EXIT_FAILURE;
    
    if (!config.InitParameters())
        return EXIT_FAILURE;
    
    const Args& args = config.args();
    config.set_env(BaseEnv::mainnet);
    if (args.IsArgSet(GLOBAL_OPTION_TESTNET))
        config.set_env(BaseEnv::testnet);
    else if (args.IsArgSet(GLOBAL_OPTION_REGTEST))
        config.set_env(BaseEnv::regtest);
    
    FullNodeMain fullnode(config);
    bool ret = false;
    try {
        ret = fullnode.Init();
        if (ret)
            ret = fullnode.Start();
    }
    catch (const Exception& e) {
        fprintf(stderr, "Error: %s\n", e.what());
    }
    catch (const std::exception& e) {
        fprintf(stderr, "Error: %s\n", e.what());
    }
    catch (...) {
        
    }

    if (ret) {
        fullnode.WaitForSignal();
    }
    
    fullnode.Interrupt();
    fullnode.Stop();
    
    //Block genesis = CreateGenesisBlock(1231006505, 2083236893, 0x1d00ffff, 1, 50*satoshi_per_bitcoin);
    //std::cout << genesis.ToString() << std::endl;
    
    return EXIT_SUCCESS;
}
