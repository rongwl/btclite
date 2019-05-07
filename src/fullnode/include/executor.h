#ifndef BTCLITE_FULLNODE_EXECUTOR_H
#define BTCLITE_FULLNODE_EXECUTOR_H


#include "chain/include/params.h"
#include "fullnode/include/config.h"
#include "p2p.h"

class FullNodeMain : public Executor {
public:
    FullNodeMain(int argc, const char* const argv[])
        : Executor(argc, argv) , data_files_(), chain_params_(), network_() {}

    //-------------------------------------------------------------------------
    bool Init();
    bool Start();
    bool Run();
    void Interrupt();
    void Stop();
    
    //-------------------------------------------------------------------------
    const FullNodeArgs& args() const
    {
        return args_;
    }
    
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
