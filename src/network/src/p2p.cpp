#include "p2p.h"
#include "fullnode/include/config.h"


namespace btclite {
namespace network {


bool P2P::Init(const chain::ChainState& chain_state)
{
    BTCLOG(LOG_LEVEL_INFO) << "Initializing p2p network...";
    
    if (params_.discover_local_addr()) {
        local_service_.DiscoverLocalAddrs();
    }
    
    Nodes nodes;
    static Context ctx = { &params_, &local_service_, &nodes, 
                           const_cast<chain::ChainState*>(&chain_state),
                           &ban_list_, &peers_ };
    if (!acceptor_.InitEvent(&ctx))
        return false;
    
    if (!connector_.InitEvent())
        return false;
    
    if (peers_db_.LoadPeers(&peers_)) {
        BTCLOG(LOG_LEVEL_INFO) << "Loaded " << peers_.Size() 
                               << " addresses from peers.dat";
    }
    else {
        BTCLOG(LOG_LEVEL_INFO) << "Invalid or missing peers.dat; recreating";
        peers_.Clear();
        peers_db_.DumpPeers(peers_);
    }
    
    if (ban_db_.LoadBanList(&ban_list_)) {
        ban_list_.SweepBanned(); // sweep out unused entries
        BTCLOG(LOG_LEVEL_INFO) << "Loaded " << ban_list_.Size() 
                               << " banned node ips/subnets from banlist.dat";
    }
    else {
        BTCLOG(LOG_LEVEL_INFO) << "Invalid or missing banlist.dat; recreating";
        ban_db_.DumpBanList(ban_list_);
    }
    
    BTCLOG(LOG_LEVEL_INFO) << "Finished initializing p2p network.";
    
    return true;
}

bool P2P::Start(const chain::ChainState& chain_state)
{    
    BTCLOG(LOG_LEVEL_INFO) << "Starting p2p network...";
    
    util::SingletonInterruptor::GetInstance().Reset();
    
    // start acceptor
    thread_acceptor_loop_ = std::thread(&util::TraceThread<std::function<void()> >, 
                                        "acceptor",
                                        std::function<void()>(std::bind(
                                                &Acceptor::StartEventLoop, &acceptor_)));
    
    // start connector
    thread_connector_loop_ = std::thread(&util::TraceThread<std::function<void()> >, 
                                         "connector",
                                         std::function<void()>(std::bind(
                                                 &Connector::StartEventLoop, &connector_)));
    
    Nodes nodes;
    static Context ctx = { &params_, &local_service_, &nodes, 
                                  const_cast<chain::ChainState*>(&chain_state),
                                  &ban_list_, &peers_ };
    if (!params_.specified_outgoing().empty()) {
        if (!connector_.ConnectNodes(params_.specified_outgoing(), 
                                     ctx, true)) {
            BTCLOG(LOG_LEVEL_ERROR) << "Connecting specified outgoing failed.";
            return false;
        }
    }
    else  {
        if (!connector_.StartOutboundTimer(&ctx)) {
            BTCLOG(LOG_LEVEL_ERROR) << "Starting outbound timer failed.";
            return false;
        }
    }
    
    BTCLOG(LOG_LEVEL_INFO) << "Finished starting p2p network.";

    return true;
}

void P2P::Interrupt()
{
    BTCLOG(LOG_LEVEL_INFO) << "Interrupting p2p network...";
    
    acceptor_.ExitEventLoop();
    connector_.ExitEventLoop();    
    
    BTCLOG(LOG_LEVEL_INFO) << "Finished interrupting p2p network.";
}

void P2P::Stop()
{
    BTCLOG(LOG_LEVEL_INFO) << "Stoping p2p network...";
    
    if (thread_acceptor_loop_.joinable())
        thread_acceptor_loop_.join();
    
    if (thread_connector_loop_.joinable())
        thread_connector_loop_.join();
    
    peers_db_.DumpPeers(peers_);
    ban_db_.DumpBanList(ban_list_);
    
    BTCLOG(LOG_LEVEL_INFO) << "Finished stoping p2p network.";
}

} // namespace network
} // namespace btclite
