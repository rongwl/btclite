#include "p2p.h"
#include "fullnode/include/config.h"


bool P2P::Init()
{
    BTCLOG(LOG_LEVEL_INFO) << "Initializing p2p network...";
    
    if (!acceptor_.InitEvent())
        return false;
    
    if (ban_db_.LoadBanList()) {
        ban_db_.set_dirty(false); // no need to write down, just read data
        ban_db_.SweepBanned(); // sweep out unused entries
        BTCLOG(LOG_LEVEL_INFO) << "Load " << ban_db_.Size() << " banned node ips/subnets from banlist.dat";
    }
    else {
        BTCLOG(LOG_LEVEL_INFO) << "Invalid or missing banlist.dat; recreating";
        ban_db_.set_dirty(false); // force write
        ban_db_.DumpBanList();
    }
    
    BTCLOG(LOG_LEVEL_INFO) << "Initialized p2p network.";
    
    return true;
}

bool P2P::Start()
{   
    BTCLOG(LOG_LEVEL_INFO) << "Starting p2p network...";
    
    interrupt_.Reset();
    
    thread_acceptor_loop_ = std::thread(&TraceThread<std::function<void()> >, "acceptor",
                                        std::function<void()>(std::bind(&Acceptor::StartEventLoop, &acceptor_)));
    
    BTCLOG(LOG_LEVEL_INFO) << "Started p2p network.";

    return true;
}

void P2P::Interrupt()
{
    BTCLOG(LOG_LEVEL_INFO) << "Interrupting p2p network...";
    
    interrupt_();
    acceptor_.ExitEventLoop();
    
    BTCLOG(LOG_LEVEL_INFO) << "Interrupted p2p network.";
}

void P2P::Stop()
{
    BTCLOG(LOG_LEVEL_INFO) << "Stoping p2p network...";
    
    if (thread_acceptor_loop_.joinable())
        thread_acceptor_loop_.join();
    
    BTCLOG(LOG_LEVEL_INFO) << "Stopped p2p network.";
}

void P2P::ThreadDnsSeeds()
{

}

void P2P::ThreadOpenConnections(const std::vector<std::string> connect)
{

}

void P2P::ThreadSocketHandler()
{
    while (!interrupt_) {

    }
}

void P2P::ThreadMessageHandler()
{

}
