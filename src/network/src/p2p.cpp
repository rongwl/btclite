#include "p2p.h"
#include "fullnode/include/config.h"
#include "net.h"


namespace btclite {
namespace network {

P2P::P2P(const util::Configuration& config)
    : params_(config), 
      peers_db_(config.path_data_dir()),
      ban_db_(config.path_data_dir()),
      acceptor_(params_), connector_(params_)
{
    //SingletonLocalService::GetInstance();
}

bool P2P::Init()
{
    BTCLOG(LOG_LEVEL_INFO) << "Initializing p2p network...";
    
    if (params_.discover_local_addr()) {
        SingletonLocalService::GetInstance().DiscoverLocalAddrs();
    }
    
    if (!acceptor_.InitEvent())
        return false;
    
    if (!connector_.InitEvent())
        return false;
    
    Peers& peers = SingletonPeers::GetInstance();
    if (peers_db_.LoadPeers(&peers)) {
        BTCLOG(LOG_LEVEL_INFO) << "Loaded " << peers.Size() 
                               << " addresses from peers.dat";
    }
    else {
        BTCLOG(LOG_LEVEL_INFO) << "Invalid or missing peers.dat; recreating";
        SingletonPeers::GetInstance().Clear();
        peers_db_.DumpPeers(SingletonPeers::GetInstance());
    }
    
    BanList& ban_list = SingletonBanList::GetInstance();
    if (ban_db_.LoadBanList(&ban_list)) {
        ban_list.SweepBanned(); // sweep out unused entries
        BTCLOG(LOG_LEVEL_INFO) << "Loaded " << ban_list.Size() 
                               << " banned node ips/subnets from banlist.dat";
    }
    else {
        BTCLOG(LOG_LEVEL_INFO) << "Invalid or missing banlist.dat; recreating";
        ban_db_.DumpBanList(ban_list);
    }
    
    BTCLOG(LOG_LEVEL_INFO) << "Initialized p2p network.";
    
    return true;
}

bool P2P::Start()
{    
    BTCLOG(LOG_LEVEL_INFO) << "Starting p2p network...";
    
    SingletonNetInterrupt::GetInstance().Reset();
    
    // start acceptor
    thread_acceptor_loop_ = std::thread(&util::TraceThread<std::function<void()> >, "acceptor",
                                        std::function<void()>(std::bind(&Acceptor::StartEventLoop, &acceptor_)));
    
    // start connector
    thread_connector_loop_ = std::thread(&util::TraceThread<std::function<void()> >, "connector",
                                         std::function<void()>(std::bind(&Connector::StartEventLoop, &connector_)));
    if (!params_.specified_outgoing().empty()) {
        if (!connector_.ConnectNodes(params_.specified_outgoing(), true)) {
            BTCLOG(LOG_LEVEL_ERROR) << "Connecting specified outgoing failed.";
            return false;
        }
    }
    else  {
        if (!connector_.StartOutboundTimer()) {
            BTCLOG(LOG_LEVEL_ERROR) << "Starting outbound timer failed.";
            return false;
        }
    }
    
    BTCLOG(LOG_LEVEL_INFO) << "Started p2p network.";

    return true;
}

void P2P::Interrupt()
{
    BTCLOG(LOG_LEVEL_INFO) << "Interrupting p2p network...";
    
    SingletonNetInterrupt::GetInstance().Interrupt();
    util::SingletonTimerMng::GetInstance().set_stop(true);
    acceptor_.ExitEventLoop();
    connector_.ExitEventLoop();    
    
    BTCLOG(LOG_LEVEL_INFO) << "Interrupted p2p network.";
}

void P2P::Stop()
{
    BTCLOG(LOG_LEVEL_INFO) << "Stoping p2p network...";
    
    if (thread_acceptor_loop_.joinable())
        thread_acceptor_loop_.join();
    
    if (thread_connector_loop_.joinable())
        thread_connector_loop_.join();
    
    peers_db_.DumpPeers(SingletonPeers::GetInstance());
    ban_db_.DumpBanList(SingletonBanList::GetInstance());
    
    BTCLOG(LOG_LEVEL_INFO) << "Stopped p2p network.";
}

} // namespace network
} // namespace btclite
