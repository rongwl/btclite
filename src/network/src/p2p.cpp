#include "p2p.h"
#include "fullnode/include/config.h"


namespace btclite {
namespace network {

P2P::P2P(const ExecutorConfig& config)
    : peers_db_(config.path_data_dir())
{
    btclite::network::SingletonParams::GetInstance(config.env());
    SingletonNetArgs::GetInstance(config.args());
    //SingletonLocalNetCfg::GetInstance();
    //SingletonNodes::GetInstance();
    SingletonBanDb::GetInstance(config.path_data_dir());
    //SingletonNetInterrupt::GetInstance();
}

bool P2P::Init()
{
    BanDb& ban_db = SingletonBanDb::GetInstance();
    
    BTCLOG(LOG_LEVEL_INFO) << "Initializing p2p network...";
    
    if (!acceptor_.InitEvent())
        return false;
    
    if (!connector_.InitEvent())
        return false;
    
    if (peers_db_.LoadPeers()) {
        BTCLOG(LOG_LEVEL_INFO) << "Loaded " << peers_db_.Size() << " addresses from peers.dat";
    }
    else {
        BTCLOG(LOG_LEVEL_INFO) << "Invalid or missing peers.dat; recreating";
        SingletonPeers::GetInstance().Clear();
        peers_db_.DumpPeers();
    }
    
    if (ban_db.LoadBanList()) {
        ban_db.set_dirty(false); // no need to write down, just read data
        ban_db.SweepBanned(); // sweep out unused entries
        BTCLOG(LOG_LEVEL_INFO) << "Loaded " << ban_db.Size() << " banned node ips/subnets from banlist.dat";
    }
    else {
        BTCLOG(LOG_LEVEL_INFO) << "Invalid or missing banlist.dat; recreating";
        ban_db.set_dirty(true); // force write
        ban_db.DumpBanList();
    }
    
    BTCLOG(LOG_LEVEL_INFO) << "Initialized p2p network.";
    
    return true;
}

bool P2P::Start()
{   
    NetArgs& net_args = SingletonNetArgs::GetInstance();
    
    BTCLOG(LOG_LEVEL_INFO) << "Starting p2p network...";
    
    SingletonNetInterrupt::GetInstance().Reset();
    
    // start acceptor
    thread_acceptor_loop_ = std::thread(&TraceThread<std::function<void()> >, "acceptor",
                                        std::function<void()>(std::bind(&Acceptor::StartEventLoop, &acceptor_)));
    
    // start connector
    thread_connector_loop_ = std::thread(&TraceThread<std::function<void()> >, "connector",
                                         std::function<void()>(std::bind(&Connector::StartEventLoop, &connector_)));
    if (!net_args.specified_outgoing().empty()) {
        if (!connector_.ConnectNodes(net_args.specified_outgoing()), true)
            return false;
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
    
    SingletonNetInterrupt::GetInstance()();
    SingletonTimerMng::GetInstance().set_stop(true);
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
    
    peers_db_.DumpPeers();
    SingletonBanDb::GetInstance().DumpBanList();
    
    BTCLOG(LOG_LEVEL_INFO) << "Stopped p2p network.";
}

} // namespace network
} // namespace btclite
