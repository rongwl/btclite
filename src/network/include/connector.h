#ifndef BTCLITE_CONNECTOR_H
#define BTCLITE_CONNECTOR_H


#include "libevent.h"


namespace btclite {
namespace network {

// outbound socket connection
class Connector {
public:    
    Connector(const Params params)
        : params_(params), base_(nullptr), bev_(nullptr), 
          outbound_timer_(nullptr) {}
    
    ~Connector()
    {
        if (base_)
            event_base_free(base_);
    }    
    
    //-------------------------------------------------------------------------
    bool InitEvent();
    void StartEventLoop();
    void ExitEventLoop();
    struct bufferevent *NewSocketEvent(const Context *ctx);
    
    //-------------------------------------------------------------------------
    bool StartOutboundTimer(const LocalService& local_service,
                            const Peers& peers, const BanList& banlist);
    bool OutboundTimeOutCb(const LocalService& local_service,
                           const BanList& banlist, Peers *ppeers);
    bool ConnectNodes(const std::vector<std::string>& str_addrs, 
                      const LocalService& local_service,
                      const BanList& banlist, Peers *ppeers,
                      bool manual = false);
    bool ConnectNodes(const std::vector<NetAddr>& addrs, 
                      const LocalService& local_service,
                      const BanList& banlist, Peers *ppeers,
                      bool manual = false);
    bool GetHostAddr(const std::string& host_name, NetAddr *out);
    bool DnsLookup(const std::vector<Seed>& seeds, uint16_t default_port,
                   Peers *ppeers);

    //-------------------------------------------------------------------------
    const Nodes& outbounds() const
    {
        return outbounds_;
    }
    
private:
    const Params params_;
    
    struct event_base *base_;
    struct bufferevent *bev_;
    util::TimerMng::TimerPtr outbound_timer_;
    
    Nodes outbounds_;
    
    bool ConnectNode(const NetAddr& addr, const LocalService& local_service,
                     const BanList& ban_list, Peers *ppeers, bool manual = false);
};

} // namespace network
} // namespace btclite

#endif //BTCLITE_CONNECTOR_H
