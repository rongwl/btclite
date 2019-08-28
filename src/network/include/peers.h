#ifndef BTCLITE_PEERS_H
#define BTCLITE_PEERS_H


#include "arithmetic.h"
#include "network_address.h"
#include "peers.pb.h"
#include "random.h"
#include "sync.h"
#include "utiltime.h"


namespace btclite {


class Peers {
public:
    Peers()
        : key_(std::move(Random::GetUint256()))
    {
        proto_peers_.mutable_key()->Resize(4, 0);
        std::memcpy(proto_peers_.mutable_key()->mutable_data(), key_.data(), key_.Size());
    }
    
    // Add a single address.
    bool Add(const btclite::NetAddr &addr, const btclite::NetAddr& source, int64_t time_penalty = 0);
    
    //! Add multiple addresses.
    bool Add(const std::vector<btclite::NetAddr> &vAddr, const btclite::NetAddr& source, int64_t time_penalty = 0);
    
    // Move address to the tried table.
    bool MakeTried(const btclite::NetAddr& addr, int64_t time = SingletonTime::GetInstance().GetAdjustedTime());
    
    // Choose an address to connect to.
    bool Select(btclite::NetAddr *out, bool newOnly = false);
    
    // Find an entry.
    bool Find(uint64_t map_key, uint64_t group_key, proto_peers::Peer *out, bool *is_new, bool *is_tried);
    bool Find(const btclite::NetAddr& addr, proto_peers::Peer *out, bool *is_new, bool *is_tried);
    bool FindSameGroup(uint64_t group_key, proto_peers::Peer *out, bool *is_tried, uint64_t *key = nullptr);
    
    // Mark an entry as connection attempted to.
    bool Attempt(const btclite::NetAddr &addr, int64_t time = SingletonTime::GetInstance().GetAdjustedTime());
    
    //! Update time of the address.
    bool UpdateTime(const btclite::NetAddr &addr, int64_t time = SingletonTime::GetInstance().GetAdjustedTime());
    
    // Return a bunch of addresses, selected at random.
    bool GetAddrs(std::vector<btclite::NetAddr> *out);
    
    uint64_t MakeMapKey(const btclite::NetAddr& addr, bool by_group = false);
    
    static bool IsTerrible(const proto_peers::Peer& peer, 
                    int64_t now = SingletonTime::GetInstance().GetAdjustedTime());
    
    void Clear();
        
    //-------------------------------------------------------------------------
    proto_peers::Peers proto_peers() const
    {
        LOCK(cs_peers_);
        return proto_peers_;
    }
    
    std::vector<uint64_t> rand_order_keys() const
    {
        LOCK(cs_peers_);
        return rand_order_keys_;
    }
    
    const Uint256& key() const
    {
        return key_;
    }

private:
    static constexpr uint32_t max_new_tbl_size = 1024*64;
    static constexpr uint32_t max_tried_tbl_size = 256*64;
    
    // critical section to protect the inner data structures
    mutable CriticalSection cs_peers_;
    proto_peers::Peers proto_peers_;
    
    // randomly-ordered vector of all map_keys
    std::vector<uint64_t> rand_order_keys_;
    
    // clone of the peers_.key for writing hash stream
    const Uint256 key_;
    
    void EraseRand(uint64_t key);
};


} //namespace btclite

class SingletonPeers : Uncopyable {
public:
    static btclite::Peers& GetInstance()
    {
        static btclite::Peers peers;
        return peers;
    }
    
private:
    SingletonPeers() {}
};

#endif // BTCLITE_PEERS_H
