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
        : local_services_(ServiceFlags(kNodeNetwork | kNodeNetworkLimited)), local_addrs_() {}
    
    bool LookupLocalAddrs();
    bool IsLocal(const btclite::network::NetAddr& addr);
    
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
    
    std::vector<btclite::network::NetAddr> local_addrs() const // thread safe copy
    {
        LOCK(cs_local_net_config_);
        return local_addrs_;
    }
    
private:
    mutable CriticalSection cs_local_net_config_;
    ServiceFlags local_services_;
    std::vector<btclite::network::NetAddr> local_addrs_;
    
    bool AddLocalHost(const btclite::network::NetAddr& addr);
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
    
    bool is_listen() const
    {
        return is_listen_;
    }
    
    bool is_discover() const
    {
        return is_discover_;
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
    bool is_listen_;
    bool is_discover_;
    bool is_dnsseed_;
    std::vector<std::string> specified_outgoing_;
};


#endif // BTCLITE_NET_H
