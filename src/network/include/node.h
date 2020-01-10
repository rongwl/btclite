#ifndef BTCLITE_NODE_H
#define BTCLITE_NODE_H


#include <boost/circular_buffer.hpp>
#include <event2/bufferevent.h>
#include <functional>
#include <queue>

#include "bloom.h"
#include "block_sync.h"
#include "timer.h"


namespace btclite {
namespace network {

enum ProtocolVersion : uint32_t {
    kUnknownProtoVersion = 0,
    
    // initial proto version, to be increased after version/verack negotiation
    kInitProtoVersion = 209,
    
    // timestamp field added to NetAddr, starting with this version;
    // if possible, avoid requesting addresses nodes older than this
    kAddrTimeVersion = 31402,
    
    // In this version, 'getheaders' was introduced.
    kGetheadersVersion = 31800,
    
    // BIP 0031, pong message, is enabled for all versions AFTER this one
    kBip31Version = 60000,
    
    // BIP 0037, whether the remote peer should announce relayed transactions or not
    kRelayedTxsVersion = 70001,
    
    // "filter*" commands are disabled without kNodeBloom after and including this version
    kNoBloomVersion = 70011,
    
    // "sendheaders" command and announcing blocks with headers starts with this version
    kSendheadersVersion = 70012,
    
    // "feefilter" tells peers to filter invs to you by fee starts with this version
    kShortIdsBlocksVersion = 70014,
    
    // not banning for invalid compact blocks starts with this version
    kInvalidCbNoBanVersion = 70015
};

/* Services flags */
enum ServiceFlags : uint64_t {
    // Nothing
    kNodeNone = 0,
    
    // kNodeNetwork means that the node is capable of serving the complete block chain. It is currently
    // set by all Bitcoin Core non pruned nodes, and is unset by SPV clients or other light clients.
    kNodeNetwork = (1 << 0),
    
    // kNodeGetutxo means the node is capable of responding to the getutxo protocol request.
    // Bitcoin Core does not support this but a patch set called Bitcoin XT does.
    // See BIP 64 for details on how this is implemented.
    kNodeGetutxo = (1 << 1),
    
    // kNodeBloom means the node is capable and willing to handle bloom-filtered connections.
    // Bitcoin Core nodes used to support this by default, without advertising this bit,
    // but no longer do as of protocol version 70011 (= NO_BLOOM_VERSION)
    kNodeBloom = (1 << 2),
    
    // kNodeWitness indicates that a node can be asked for blocks and transactions including
    // witness data.
    kNodeWitness = (1 << 3),
    
    // kNodeXthin means the node supports Xtreme Thinblocks
    // If this is turned off then the node will not service nor make xthin requests
    kNodeXthin = (1 << 4),
    
    // kNodeNetworkLimited means the same as kNodeNetwork with the limitation of only
    // serving the last 288 (2 day) blocks
    // See BIP159 for details on how this is implemented.
    kNodeNetworkLimited = (1 << 10),

    // Bits 24-31 are reserved for temporary experiments. Just pick a bit that
    // isn't getting used, or one not being used much, and notify the
    // bitcoin-development mailing list. Remember that service bits are just
    // unauthenticated advertisements, so your code must be robust against
    // collisions and other cases where nodes may be advertising a service they
    // do not actually support. Other service bits should be allocated via the
    // BIP process.
};

constexpr uint64_t kDesirableServiceFlags = (kNodeNetwork | kNodeWitness);

/**
 * A shortcut for (services & kDesirableServiceFlags)
 * == kDesirableServiceFlags, ie determines whether the given
 * set of service flags are sufficient for a peer to be "relevant".
 */
static inline bool IsServiceFlagDesirable(uint64_t services) {
    return !(kDesirableServiceFlags & (~services));
}

class NodeProtocol {
public:
    bool IsClient() const
    {
        return !(services_ & kNodeNetwork);
    }
    
    ProtocolVersion version() const
    {
        return version_;
    }
    
    void set_version(ProtocolVersion version)
    {
        version_ = version;
    }
    
    ServiceFlags services() const
    {
        return services_;
    }
    
    void set_services(ServiceFlags services)
    {
        services_ = services;
    }
    
    int start_height() const
    {
        return start_height_;
    }
    
    void set_start_height(int start_height)
    {
        start_height_ = start_height;
    }
    
private:
    std::atomic<ProtocolVersion> version_ = kUnknownProtoVersion;
    std::atomic<ServiceFlags> services_ = kNodeNone;
    std::atomic<int> start_height_ = 0;
};

class NodeConnection {
public:
    NodeConnection(NodeId id, const struct bufferevent *bev, const NetAddr& addr,
                   bool is_inbound, bool manual, std::string host_name)
        : node_id_(id), bev_(const_cast<struct bufferevent*>(bev)), addr_(addr),
          host_name_(host_name), is_inbound_(is_inbound), manual_(manual) {}
    
    ~NodeConnection()
    {
        if (bev_)
            bufferevent_free(bev_);
    }
    
    const struct bufferevent* const bev() const
    {
        return bev_;
    }
    
    struct bufferevent *mutable_bev()
    {
        return bev_;
    }
    
    const NetAddr& addr() const
    {
        return addr_;
    }
    
    std::string host_name() const // thread safe copy
    {
        LOCK(cs_host_name_);
        return host_name_;
    }
    
    void set_host_name(const std::string& name)
    {
        LOCK(cs_host_name_);
        host_name_ = name;
    }
    
    NetAddr local_addr() const // thread safe copy
    {
        LOCK(cs_local_addr_);
        return local_addr_;
    }
    
    void set_local_addr(const NetAddr& addr)
    {
        LOCK(cs_local_addr_);
        if (!local_addr_.IsValid())
            local_addr_ = addr;
    }
    
    bool is_inbound() const
    {
        return is_inbound_;
    }
    
    bool manual() const
    {
        return manual_;
    }
    
    bool established() const
    {
        return established_;
    }
    
    void set_established(bool established)
    {
        established_ = established;
    }
    
    bool disconnected() const
    {
        return disconnected_;
    }
    
    void set_disconnected(bool disconnected);
    
private:
    const NodeId node_id_;
    struct bufferevent *bev_ = nullptr;    
    const NetAddr addr_;
    //const uint64_t keyed_net_group_;  
    
    mutable util::CriticalSection cs_host_name_;
    std::string host_name_;
    
    mutable util::CriticalSection cs_local_addr_;
    NetAddr local_addr_;    
    
    const bool is_inbound_;
    const bool manual_;
    std::atomic<bool> established_ = false;
    std::atomic<bool> disconnected_ = false;
};

class BroadcastAddrs {
public:
    BroadcastAddrs()
        : known_addrs_(3000) {}
    
    //-------------------------------------------------------------------------
    bool PushAddrToSend(const NetAddr& addr);    
    void ClearSentAddr();    
    bool AddKnownAddr(const NetAddr& addr);
    
    bool IsKnownAddr(const NetAddr& addr) const
    {
        LOCK(cs_addrs_);
        return (std::find(known_addrs_.begin(), known_addrs_.end(), 
                          addr.GetHash().GetLow64()) != known_addrs_.end());
    }
    
    //-------------------------------------------------------------------------
    std::vector<NetAddr> addrs_to_send() const
    {
        LOCK(cs_addrs_);
        return addrs_to_send_;
    }
    
    bool sent_getaddr() const
    {        
        return sent_getaddr_;
    }
    
    void set_sent_getaddr(bool sent_getaddr)
    {
        sent_getaddr_ = sent_getaddr;
    }
    
private:
    mutable util::CriticalSection cs_addrs_;
    std::vector<NetAddr> addrs_to_send_;
    boost::circular_buffer<uint64_t> known_addrs_;
    bool sent_getaddr_ = false;
};

class NodeFilter {
public:
    bool relay_txes() const
    {
        LOCK(cs_bloom_filter_);
        return relay_txes_;
    }
    
    void set_relay_txes(bool relay_txes)
    {
        LOCK(cs_bloom_filter_);
        relay_txes_ = relay_txes;
    }
    
    const BloomFilter *bloom_filter() const;
    
private:
    mutable util::CriticalSection cs_bloom_filter_;
    
    // We use fRelayTxes for two purposes -
    // a) it allows us to not relay tx invs before receiving the peer's version message
    // b) the peer may tell us in its version message that we should not relay tx invs
    //    unless it loads a bloom filter.
    bool relay_txes_ = false; 
    std::unique_ptr<BloomFilter> bloom_filter_ = nullptr;
};

// Ping time measurement
struct PingTime {
    // The pong reply we're expecting, or 0 if no pong expected.
    std::atomic<uint64_t> ping_nonce_sent = 0;
    
    // Time (in usec) the last ping was sent, or 0 if no ping was ever sent.
    std::atomic<int64_t> ping_usec_start = 0;
    
    // Last measured round-trip time.
    std::atomic<int64_t> ping_usec_time = 0;
    
    // Best measured round-trip time.
    std::atomic<int64_t> min_ping_usec_time = 0;
    
    // Whether a ping is requested.
    std::atomic<bool> ping_queued = false;
};

struct NodeTime {
    NodeTime(int64_t time)
        : time_connected(time) {}
    
    const int64_t time_connected;
    
    std::atomic<int64_t> time_last_send = 0;
    std::atomic<int64_t> time_last_recv = 0;
    
    // Block and TXN accept times
    std::atomic<int64_t> time_last_block = 0;
    std::atomic<int64_t> time_last_tx = 0;
    
    // Ping time measurement
    PingTime ping_time;
};

struct NodeTimers {
    util::TimerMng::TimerPtr no_msg_timer;
    util::TimerMng::TimerPtr no_sending_timer;
    util::TimerMng::TimerPtr no_receiving_timer;
    util::TimerMng::TimerPtr no_connection_timer;
    
    util::TimerMng::TimerPtr ping_timer;
    util::TimerMng::TimerPtr broadcast_local_addr_timer;
    util::TimerMng::TimerPtr broadcast_addr_timer;
};

/* Information about a connected peer */
class Node : util::Uncopyable {
public:
    Node(const struct bufferevent *bev, const NetAddr& addr,
         bool is_inbound = true, bool manual = false, 
         std::string host_name = "");    
    ~Node();
    
    //-------------------------------------------------------------------------
    static void InactivityTimeoutCb(std::shared_ptr<Node> node);
    bool CheckBanned();
    
    //-------------------------------------------------------------------------   
    NodeId id() const
    {
        return id_;
    }
    
    uint64_t local_host_nonce() const
    {
        return local_host_nonce_;
    }
    
    const NodeProtocol& protocol() const
    {
        return protocol_;
    }
    
    NodeProtocol *mutable_protocol()
    {
        return &protocol_;
    }
    
    const NodeConnection& connection() const
    {
        return connection_;
    }
    
    NodeConnection *mutable_connection()
    {
        return &connection_;
    }
    
    /*uint64_t keyed_net_group() const
    {
        return keyed_net_group_;
    }*/
    
    const BroadcastAddrs& broadcast_addrs() const
    {
        return broadcast_addrs_;
    }
    
    BroadcastAddrs *mutable_broadcast_addrs()
    {
        return &broadcast_addrs_;
    }
    
    const NodeFilter& filter() const
    {
        return filter_;
    }
    
    NodeFilter *mutable_filter()
    {
        return &filter_;
    }
    
    const NodeTime& time() const
    {
        return time_;
    }
    
    NodeTime *mutable_time()
    {
        return &time_;
    }
    
    const NodeTimers& timers() const
    {
        return timers_;
    }
    
    NodeTimers *mutable_timers()
    {
        return &timers_;
    }
    
private:
    const NodeId id_;
    const uint64_t local_host_nonce_;  
    NodeProtocol protocol_;
    NodeConnection connection_;
    //const uint64_t keyed_net_group_;  

    // flood addrs relay
    BroadcastAddrs broadcast_addrs_;
    
    NodeFilter filter_;    
    NodeTime time_;    
    NodeTimers timers_;
};

struct NodeEvictionCandidate
{
    NodeId id;
    int64_t time_connected;
    int64_t min_ping_usec_time;
    int64_t last_block_time;
    int64_t last_tx_time;
    bool relevant_services;
    bool relay_txes;
    bool bloom_filter;
    NetAddr addr;
    uint64_t keyed_net_group;
};

class Nodes : util::Uncopyable {
public:
    Nodes()
        : list_() {}
    
    NodeId GetNewNodeId()
    {
        static std::atomic<NodeId> last_node_id = 0;
        return last_node_id.fetch_add(1, std::memory_order_relaxed);
    }
    
    size_t Size() const
    {
        LOCK(cs_nodes_);
        return list_.size();
    }
    
    //-------------------------------------------------------------------------
    void AddNode(std::shared_ptr<Node> node);    
    std::shared_ptr<Node> InitializeNode(const struct bufferevent *bev, const NetAddr& addr,
                                         bool is_inbound = true, bool manual = false);
    std::shared_ptr<Node> GetNode(NodeId id);    
    std::shared_ptr<Node> GetNode(struct bufferevent *bev);    
    std::shared_ptr<Node> GetNode(const NetAddr& addr);    
    void EraseNode(std::shared_ptr<Node> node);    
    void EraseNode(NodeId id);
    
    //-------------------------------------------------------------------------
    void Clear()
    {
        LOCK(cs_nodes_);
        list_.clear();
    }
    
    void ClearDisconnected();    
    void DisconnectNode(const SubNet& subnet);
    
    //-------------------------------------------------------------------------
    //bool AttemptToEvictConnection();
    int CountInbound();
    int CountOutbound();
    bool ShouldDnsLookup();
    bool CheckIncomingNonce(uint64_t nonce);
    
private:
    mutable util::CriticalSection cs_nodes_;
    std::list<std::shared_ptr<Node> > list_;

    //void MakeEvictionCandidate(std::vector<NodeEvictionCandidate> *out);
};


// Singleton pattern, thread safe after c++11
class SingletonNodes : util::Uncopyable {
public:
    static Nodes& GetInstance()
    {
        static Nodes nodes;
        return nodes;
    }
    
private:
    SingletonNodes() {}    
};

} // namespace network
} // namespace btclite


#endif // BTCLITE_NODE_H
