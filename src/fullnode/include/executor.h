#ifndef BTCLITE_FULLNODE_EXECUTOR_H
#define BTCLITE_FULLNODE_EXECUTOR_H

#include "fullnode/include/config.h"
#include "p2p.h"

class FullNodeMain : public Executor {
public:
    FullNodeMain(int argc, const char* const argv[])
        : Executor(argc, argv) , data_files_(FullNodeDataFiles::DefaultDataDirPath(), DEFAULT_CONFIG_FILE),
          chain_params_(), network_() {}
    
    bool Init();
    bool Start();
    bool Run();
    void Interrupt();
    void Stop();
    
private:
    FullNodeArgs args_;
    FullNodeDataFiles data_files_;
    Chain::Params chain_params_;
    P2P network_;

    bool InitDataFiles();
    bool LoadConfigFile();
    bool InitNetwork();
};


#endif // BTCLITE_FULLNODE_EXECUTOR_H
