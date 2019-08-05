#include "fullnode/include/executor.h"


bool FullNodeMain::Init()
{
    if (!BasicSetup())
        return false;

    return true;
}

bool FullNodeMain::Start()
{
    return true;
}

bool FullNodeMain::Run()
{
    return true;
}

void FullNodeMain::Interrupt()
{
    
}

void FullNodeMain::Stop()
{
    BTCLOG(LOG_LEVEL_INFO) << __func__ << ": progress...";
    
    BTCLOG(LOG_LEVEL_INFO) << __func__ << ": done";
}

bool FullNodeMain::InitNetwork()
{        
    return true;
}


