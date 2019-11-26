#ifndef BTCLITE_NET_H
#define BTCLITE_NET_H


#include "network_address.h"
#include "protocol/message.h"
#include "serialize.h"
#include "thread.h"
#include "util.h"


namespace btclite {
namespace network {

class LocalNetConfig {
public:
    LocalNetConfig()
        : local_services_(ServiceFlags(kNodeNetwork | kNodeNetworkLimited)), 
          map_local_addrs_() {}
    
    //-------------------------------------------------------------------------
    bool LookupLocalAddrs();
    bool GetLocalAddr(const NetAddr& peer_addr, ServiceFlags services, NetAddr *out);
    int GetAddrScore(const NetAddr& peer_addr);    
    
    bool IsLocal(const btclite::network::NetAddr& addr)
    {
        LOCK(cs_local_net_config_);
        return (map_local_addrs_.count(addr) > 0);
    }
    
    static void BroadcastTimeoutCb(std::shared_ptr<Node> node);
    
    //-------------------------------------------------------------------------
    ServiceFlags local_services() const
    {
        LOCK(cs_local_net_config_);
        return local_services_;
    }
    void set_local_services(ServiceFlags flags)
    {
        LOCK(cs_local_net_config_);
        local_services_ = flags;
    }
    
    std::map<btclite::network::NetAddr, int> map_local_addrs() const // thread safe copy
    {
        LOCK(cs_local_net_config_);
        return map_local_addrs_;
    }
    
private:
    mutable CriticalSection cs_local_net_config_;
    ServiceFlags local_services_;
    std::map<btclite::network::NetAddr, int> map_local_addrs_;
    
    bool AddLocalHost(const btclite::network::NetAddr& addr, int score);
};

class NetArgs {
public:
    explicit NetArgs(const Args& args);
    
    bool listening() const
    {
        return listening_;
    }
    
    bool should_discover() const
    {
        return should_discover_;
    }
    
    bool is_dnsseed() const
    {
        return is_dnsseed_;
    }
    
    const std::vector<std::string>& specified_outgoing() const
    {
        return specified_outgoing_;
    }
    
private:
    bool listening_ = true;
    bool should_discover_ = true;
    bool is_dnsseed_ = true;
    std::vector<std::string> specified_outgoing_;
};


bool IsPeerLocalAddrGood(std::shared_ptr<Node> node);
void AdvertiseLocalAddr(std::shared_ptr<Node> node);

void BroadcastAddrsTimeoutCb(std::shared_ptr<Node> node);

// Return a time interavl for send in the future (in microseconds) for exponentially distributed events.
int64_t IntervalNextSend(int average_interval_seconds);

} // namespace network
} // namespace btclite


class SingletonLocalNetCfg : Uncopyable {
public:
    static btclite::network::LocalNetConfig& GetInstance()
    {
        static btclite::network::LocalNetConfig config;
        return config;
    }
    
private:
    SingletonLocalNetCfg() {}
};

class SingletonNetInterrupt : Uncopyable {
public:
    static ThreadInterrupt& GetInstance()
    {
        static ThreadInterrupt interrupt;
        return interrupt;
    }
    
private:
    SingletonNetInterrupt() {}
};

class SingletonNetArgs : Uncopyable {
public:
    static btclite::network::NetArgs& GetInstance(const Args& args = Args())
    {
        static btclite::network::NetArgs net_args(args);
        return net_args;
    }
    
private:
    SingletonNetArgs() {}
};

#endif // BTCLITE_NET_H
