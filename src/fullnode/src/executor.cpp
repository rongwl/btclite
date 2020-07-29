#include "fullnode/include/executor.h"

#include "thread.h"


namespace btclite {
namespace fullnode {

bool FullNode::Init()
{
    BTCLOG(LOG_LEVEL_INFO) << "Initializing btc-fullnode...";
    
    if (!BasicSetup()) {
        return false;
    }
    
    if (!chain_.Init()) {
        return false;
    }
    
    if (!network_.Init(chain_.chain_state())) {
        return false;
    }

    BTCLOG(LOG_LEVEL_INFO) << "Finished initializing btc-fullnode.";
    
    return true;
}

bool FullNode::Start()
{
    BTCLOG(LOG_LEVEL_INFO) << "Starting btc-fullnode...";
    
    if (!network_.Start(chain_.chain_state()))
        return false;
    
    BTCLOG(LOG_LEVEL_INFO) << "Finished starting btc-fullnode.";
    
    return true;
}

void FullNode::Interrupt()
{
    BTCLOG(LOG_LEVEL_INFO) << "Interrupting btc-fullnode...";
    util::SingletonInterruptor::GetInstance().Interrupt();
    util::SingletonTimerMng::GetInstance().set_stop(true);
    network_.Interrupt();
    BTCLOG(LOG_LEVEL_INFO) << "Finished interrupting btc-fullnode.";
}

void FullNode::Stop()
{
    BTCLOG(LOG_LEVEL_INFO) << "Stoping btc-fullnode...";
    
    network_.Stop();
    
    BTCLOG(LOG_LEVEL_INFO) << "Finished stoping btc-fullnode.";
}

} // namespace fullnode
} // namespace btclite

