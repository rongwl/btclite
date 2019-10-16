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
        : key_(std::move(btclite::utility::random::GetUint256()))
    {
        proto_peers_.mutable_key()->Resize(4, 0);
        std::memcpy(proto_peers_.mutable_key()->begin(), key_.begin(), key_.Size());
    }
    
    //-------------------------------------------------------------------------
    // Add a single address.
    bool Add(const btclite::network::NetAddr &addr, const btclite::network::NetAddr& source, int64_t time_penalty = 0);
    
    //! Add multiple addresses.
    bool Add(const std::vector<btclite::network::NetAddr> &vAddr, const btclite::network::NetAddr& source, int64_t time_penalty = 0);
    
    // Move address to the tried table.
    bool MakeTried(const btclite::network::NetAddr& addr, int64_t time = btclite::utility::util_time::GetAdjustedTime());
    
    // Choose an address to connect to.
    bool Select(proto_peers::Peer *out, bool newOnly = false);
    
    // Find an entry.
    bool Find(uint64_t map_key, uint64_t group_key, proto_peers::Peer *out, bool *is_new, bool *is_tried);
    bool Find(const btclite::network::NetAddr& addr, proto_peers::Peer *out, bool *is_new, bool *is_tried);
    bool FindSameGroup(uint64_t group_key, proto_peers::Peer *out, bool *is_tried, uint64_t *key = nullptr);
    
    // Mark an entry as connection attempted to.
    bool Attempt(const btclite::network::NetAddr &addr, int64_t time = btclite::utility::util_time::GetAdjustedTime());
    
    //! Update time of the address.
    bool UpdateTime(const btclite::network::NetAddr &addr, int64_t time = btclite::utility::util_time::GetAdjustedTime());
    
    // Return a bunch of addresses, selected at random.
    bool GetAddrs(std::vector<btclite::network::NetAddr> *out);
    
    uint64_t MakeMapKey(const btclite::network::NetAddr& addr, bool by_group = false);
    
    static bool IsTerrible(const proto_peers::Peer& peer, 
                    int64_t now = btclite::utility::util_time::GetAdjustedTime());
    
    //-------------------------------------------------------------------------
    bool SerializeToOstream(std::ostream * output) const
    {
        LOCK(cs_peers_);
        return proto_peers_.SerializeToOstream(output);
    }
    
    bool ParseFromIstream(std::istream * input)
    {
        LOCK(cs_peers_);
        return proto_peers_.ParseFromIstream(input);
    }
    
    //-------------------------------------------------------------------------
    void Clear();
    size_t Size() const
    {
        LOCK(cs_peers_);
        return proto_peers_.map_peers().size();
    }
    
    bool IsEmpty()
    {
        LOCK(cs_peers_);
        return proto_peers_.map_peers().empty();
    }
        
    //-------------------------------------------------------------------------
    proto_peers::Peers proto_peers() const // thread safe copy
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

class PeersDb : Uncopyable {
public:
    explicit PeersDb(const fs::path& path)
        : path_peers_(path / default_peers_file),
          peers_(SingletonPeers::GetInstance()) {}
    
    bool DumpPeers();
    bool LoadPeers();
    
    size_t Size() const
    {
        return peers_.Size();
    }
    
    const fs::path& path_peers() const
    {
        return path_peers_;
    }
    
private:
    const std::string default_peers_file = "peers.dat";
    
    fs::path path_peers_;
    btclite::Peers& peers_;
};

#endif // BTCLITE_PEERS_H
