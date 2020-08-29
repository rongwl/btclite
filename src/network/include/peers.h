#ifndef BTCLITE_PEERS_H
#define BTCLITE_PEERS_H


#include "arithmetic.h"
#include "peers.pb.h"
#include "random.h"
#include "sync.h"
#include "util_time.h"


namespace btclite {
namespace network {

namespace peer {

bool IsTerriblePeer(const proto_peers::Peer& peer, 
                    int64_t now = util::GetAdjustedTime());

} // namespace peer

class Peers {
public:
    Peers();
    
    //-------------------------------------------------------------------------
    // Add a single address.
    bool Add(const NetAddr &addr, const NetAddr& source,
             int64_t time_penalty = 0);
    
    //! Add multiple addresses.
    bool Add(const std::vector<NetAddr> &vAddr, 
             const NetAddr& source, int64_t time_penalty = 0);
    
    // Move address to the tried table.
    bool MakeTried(const NetAddr& addr, 
                   int64_t time = util::GetAdjustedTime());
    
    // Choose an address to connect to.
    bool Select(proto_peers::Peer *out, bool newOnly = false) const;
    
    bool SetServices(const NetAddr& addr, uint64_t services);
    
    // Find an entry.
    bool Find(uint64_t map_key, uint64_t group_key, proto_peers::Peer *out,
              bool *is_new, bool *is_tried);
    bool Find(const NetAddr& addr, proto_peers::Peer *out,
              bool *is_new, bool *is_tried);
    bool FindSameGroup(uint64_t group_key, proto_peers::Peer *out, bool *is_tried,
                       uint64_t *key = nullptr);
    
    // Mark an entry as connection attempted to.
    bool Attempt(const NetAddr &addr, 
                 int64_t time = util::GetAdjustedTime());
    
    //! Update time of the address.
    bool UpdateTime(const NetAddr &addr, 
                    int64_t time = util::GetAdjustedTime());
    
    // Return a bunch of addresses, selected at random.
    bool GetAddrs(std::vector<NetAddr> *out);
    
    uint64_t MakeMapKey(const NetAddr& addr, bool by_group = false);
    
    //-------------------------------------------------------------------------
    bool SerializeToOstream(std::ostream * output) const;    
    bool ParseFromIstream(std::istream * input);
    
    //-------------------------------------------------------------------------
    void Clear();
    size_t Size() const;
    bool IsEmpty() const;
        
    //-------------------------------------------------------------------------
    proto_peers::Peers proto_peers() const;
    
    std::vector<uint64_t> rand_order_keys() const;
    
    util::Hash256 key() const;

private:
    static constexpr uint32_t max_new_tbl_size = 1024*64;
    static constexpr uint32_t max_tried_tbl_size = 256*64;
    
    // critical section to protect the inner data structures
    mutable util::CriticalSection cs_peers_;
    proto_peers::Peers proto_peers_;
    
    // randomly-ordered vector of all map_keys
    std::vector<uint64_t> rand_order_keys_;
    
    // clone of the peers_.key for writing hash stream
    const util::Hash256 key_;
    
    void EraseRand(uint64_t key);
};

class SingletonPeers : util::Uncopyable {
public:
    static Peers& GetInstance();
    
private:
    SingletonPeers() {}
};

class PeersDb : util::Uncopyable {
public:
    explicit PeersDb(const fs::path& path);
    
    //-------------------------------------------------------------------------
    bool DumpPeers(const Peers& peers);
    bool LoadPeers(Peers *peers);
    
    //-------------------------------------------------------------------------
    const fs::path& path_peers() const;
    
private:
    const std::string default_peers_file = "peers.dat";
    
    fs::path path_peers_;
};

} // namespace network
} // namespace btclite

#endif // BTCLITE_PEERS_H
