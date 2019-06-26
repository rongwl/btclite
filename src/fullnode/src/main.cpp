#include "error.h"
#include "executor.h"
#include "utility/include/logging.h"
//#include "block.h"


int main(int argc, char **argv)
{
    Logging fullnode_logging;
    fullnode_logging.Init(argv[0]);
    
    bool ret = false;
    try {
        FullNodeConfig config(argc, argv);
        ret = config.InitDataDir();
        if (ret)
            ret = config.InitParameters();
    }
    catch (const Exception& e) {
        if (e.code().value() != ErrorCode::show_help)
            fprintf(stderr, "%s: %s\n", argv[0], e.what());
        FullNodeHelpInfo::PrintUsage();
        exit(e.code().value());
    }
    catch (const std::exception& e) {
        fprintf(stderr, "Error: %s\n", e.what());
    }
    catch (...) {
        
    }
    
    //Block genesis = CreateGenesisBlock(1231006505, 2083236893, 0x1d00ffff, 1, 50*satoshi_per_bitcoin);
    //std::cout << genesis.ToString() << std::endl;
    //BTCLOG_MOD(LOG_LEVEL_INFO, Logging::NET) << "test";
    
    if (ret) {
        const Args& args = ExecutorConfig::args();
        BaseEnv env = BaseEnv::mainnet;
        if (args.IsArgSet(GLOBAL_OPTION_TESTNET))
            env = BaseEnv::testnet;
        else if (args.IsArgSet(GLOBAL_OPTION_REGTEST))
            env = BaseEnv::regtest;
        
        FullNodeMain fullnode(env);
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
    }
    
    return EXIT_SUCCESS;
}
