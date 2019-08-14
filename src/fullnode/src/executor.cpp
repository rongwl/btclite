#include "fullnode/include/executor.h"


bool FullNodeMain::Init()
{
    BTCLOG(LOG_LEVEL_INFO) << "Initializing btc-fullnode...";
    
    if (!BasicSetup())
        return false;

    BTCLOG(LOG_LEVEL_INFO) << "Initialized btc-fullnode.";
    
    return true;
}

bool FullNodeMain::Start()
{
    BTCLOG(LOG_LEVEL_INFO) << "Starting btc-fullnode...";
    
    if (!network_.Init())
        return false;
    
    if (!network_.Start())
        return false;
    
    BTCLOG(LOG_LEVEL_INFO) << "Started btc-fullnode.";
    
    return true;
}

void FullNodeMain::Interrupt()
{
    BTCLOG(LOG_LEVEL_INFO) << "Interrupting btc-fullnode...";
    network_.Interrupt();
    BTCLOG(LOG_LEVEL_INFO) << "Interrupted btc-fullnode.";
}

void FullNodeMain::Stop()
{
    BTCLOG(LOG_LEVEL_INFO) << "Stoping btc-fullnode...";
    
    network_.Stop();
    
    BTCLOG(LOG_LEVEL_INFO) << "Stopped btc-fullnode.";
}



