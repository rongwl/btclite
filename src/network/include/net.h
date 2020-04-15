#ifndef BTCLITE_NET_H
#define BTCLITE_NET_H


#include "network_address.h"
#include "protocol/message.h"
#include "serialize.h"
#include "thread.h"
#include "util.h"


namespace btclite {
namespace network {


void AdvertiseLocalTimeoutCb(std::shared_ptr<Node> node);
void RelayFloodingAddrsTimeoutCb(std::shared_ptr<Node> node, uint32_t magic);

// Return a time interavl for send in the future (in microseconds) for exponentially distributed events.
int64_t IntervalNextSend(int average_interval_seconds);


class LocalService {
public:
    LocalService()
        : service_(ServiceFlags(kNodeNetwork | kNodeBloom | kNodeNetworkLimited)), 
          local_addrs_() {}
    
    //-------------------------------------------------------------------------
    bool DiscoverLocalAddrs();
    bool GetLocalAddr(const NetAddr& peer_addr, ServiceFlags services, NetAddr *out);
    void AdvertiseLocalAddr(std::shared_ptr<Node> node, bool discovered_addr_first);
    bool IsLocal(const NetAddr& addr);
    
    //-------------------------------------------------------------------------
    ServiceFlags service() const
    {
        LOCK(cs_local_service_);
        return service_;
    }
    void set_services(ServiceFlags flags)
    {
        LOCK(cs_local_service_);
        service_ = flags;
    }
    
    std::vector<NetAddr> local_addrs() const // thread safe copy
    {
        LOCK(cs_local_service_);
        return local_addrs_;
    }
    
private:
    mutable util::CriticalSection cs_local_service_;
    ServiceFlags service_;
    std::vector<NetAddr> local_addrs_;
    
    bool AddLocalAddr(const NetAddr& addr);
};


class CollectionTimer : util::Uncopyable {
public:
    CollectionTimer()
        : disconnected_nodes_timer_(
              util::SingletonTimerMng::GetInstance().StartTimer(
                  60*1000, 60*1000, CheckDisconnectedNodes)) {}
    
    static void CheckDisconnectedNodes();
    
private:
    util::TimerMng::TimerPtr disconnected_nodes_timer_;
};


class SingletonLocalService : util::Uncopyable {
public:
    static LocalService& GetInstance()
    {
        static LocalService config;
        return config;
    }
    
private:
    SingletonLocalService() {}
};

class SingletonNetInterrupt : util::Uncopyable {
public:
    static util::ThreadInterrupt& GetInstance()
    {
        static util::ThreadInterrupt interrupt;
        return interrupt;
    }
    
private:
    SingletonNetInterrupt() {}
};


} // namespace network
} // namespace btclite

#endif // BTCLITE_NET_H
