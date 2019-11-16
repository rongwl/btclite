#ifndef BTCLITE_NET_H
#define BTCLITE_NET_H


#include "network_address.h"
#include "protocol/message.h"
#include "serialize.h"
#include "thread.h"
#include "util.h"


class LocalNetConfig {
public:
    LocalNetConfig()
        : local_services_(ServiceFlags(kNodeNetwork | kNodeNetworkLimited)), 
          map_local_addrs_() {}
    
    //-------------------------------------------------------------------------
    bool LookupLocalAddrs();
    bool GetLocalAddr(const btclite::network::NetAddr& peer_addr, ServiceFlags services,
                      btclite::network::NetAddr *out);
    
    bool IsLocal(const btclite::network::NetAddr& addr)
    {
        LOCK(cs_local_net_config_);
        return (map_local_addrs_.count(addr) > 0);
    }
    
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

class SingletonLocalNetCfg : Uncopyable {
public:
    static LocalNetConfig& GetInstance()
    {
        static LocalNetConfig config;
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

class SingletonNetArgs : Uncopyable {
public:
    static NetArgs& GetInstance(const Args& args = Args())
    {
        static NetArgs net_args(args);
        return net_args;
    }
    
private:
    SingletonNetArgs() {}
};

namespace btclite {
namespace network {

bool IsPeerLocalAddrGood(std::shared_ptr<Node> node);

} // namespace network
} // namespace btclite

#endif // BTCLITE_NET_H
