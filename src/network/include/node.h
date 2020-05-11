#ifndef BTCLITE_NODE_H
#define BTCLITE_NODE_H


#include <boost/circular_buffer.hpp>
#include <event2/bufferevent.h>
#include <functional>
#include <queue>

#include "bloom.h"
#include "block_chain.h"
#include "block_sync.h"
#include "net_base.h"
#include "timer.h"


namespace btclite {
namespace network {

bool IsInitialBlockDownload();

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

struct BlockReject {
    uint8_t reject_code = 0;
    std::string reject_reason;
    util::Hash256 block_hash;
};

/* Blocks that are in flight, and that are in the queue to be downloaded. */
struct QueuedBlock {
    util::Hash256 hash;
    const chain::BlockIndex *index = nullptr;
    
    // Whether this block has validated headers at the time of request.
    bool has_validated_headers;
    
    // Optional, used for CMPCTBLOCK downloads
    //std::unique_ptr<PartiallyDownloadedBlock> partialBlock;
};

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
    enum ConnectionState {
        kInitialState,
        kEstablished,
        kDisconnected
    };
    
    NodeConnection(NodeId id, const struct bufferevent *bev, const NetAddr& addr,
                   bool is_inbound, bool manual, std::string host_name)
        : node_id_(id), bev_(const_cast<struct bufferevent*>(bev)), addr_(addr),
          host_name_(host_name), is_inbound_(is_inbound), manual_(manual) {}
    
    ~NodeConnection()
    {
        if (bev_)
            bufferevent_free(bev_);
    }
    
    //-------------------------------------------------------------------------
    bool IsLocalAddrGood(bool discover_local) const
    {
        return (discover_local && addr_.IsRoutable() && local_addr_.IsRoutable());
    }
    
    bool IsHandshakeCompleted() const
    {
        return (connection_state_ == kEstablished);
    }
    
    bool IsDisconnected() const
    {
        return (connection_state_ == kDisconnected);
    }    
    
    //-------------------------------------------------------------------------
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
    
    ConnectionState connection_state() const
    {
        LOCK(cs_state_);
        return connection_state_;
    }
    
    void set_connection_state(ConnectionState state)
    {
        LOCK(cs_state_);
        connection_state_ = state;
    }
    
    bool socket_no_msg() const
    {
        return socket_no_msg_;
    }
    
    void set_socket_no_msg(bool socket_no_msg)
    {
        socket_no_msg_ = socket_no_msg;
    }
    
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
    
    mutable util::CriticalSection cs_state_;
    ConnectionState connection_state_ = ConnectionState::kInitialState;
    
    std::atomic<bool> socket_no_msg_ = true;
};

class FloodingAddrs {
public:
    FloodingAddrs()
        : known_addrs_(3000) {}
    
    //-------------------------------------------------------------------------
    bool PushAddrToSend(const NetAddr& addr);    
    void ClearSentAddr();    
    bool AddKnownAddr(const NetAddr& addr);
    
    bool IsKnownAddr(const NetAddr& addr) const
    {
        LOCK(cs_);
        return (std::find(known_addrs_.begin(), known_addrs_.end(), 
                          addr.GetHash().GetLow64()) != known_addrs_.end());
    }
    
    //-------------------------------------------------------------------------
    std::vector<NetAddr> addrs_to_send() const // thread safe copy
    {
        LOCK(cs_);
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
    mutable util::CriticalSection cs_;
    std::vector<NetAddr> addrs_to_send_;
    boost::circular_buffer<uint64_t> known_addrs_;
    bool sent_getaddr_ = false;
};

class NodeFilter {
public:
    bool relay_txes() const
    {
        LOCK(cs_);
        return relay_txes_;
    }
    
    void set_relay_txes(bool relay_txes)
    {
        LOCK(cs_);
        relay_txes_ = relay_txes;
    }
    
    const BloomFilter *bloom_filter() const;
    
private:
    mutable util::CriticalSection cs_;
    
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
    
    //! When to potentially disconnect peer for stalling headers download
    std::atomic<int64_t> headers_sync_timeout = 0;
    
    //! Since when we're stalling block download progress (in microseconds), or 0.
    std::atomic<int64_t> stalling_since = 0;
    
    //! Time of last new block announcement
    std::atomic<int64_t> last_block_announcement = 0;
};

struct NodeTimers {
    util::TimerMng::TimerPtr no_msg_timer;    
    util::TimerMng::TimerPtr no_sending_timer;
    util::TimerMng::TimerPtr no_receiving_timer;
    util::TimerMng::TimerPtr shakehands_timer;
    
    util::TimerMng::TimerPtr ping_timer;
    util::TimerMng::TimerPtr advertise_local_addr_timer;
    util::TimerMng::TimerPtr broadcast_addrs_timer;
};

class Misbehavior {
public:    
    void Misbehaving(NodeId id, int howmuch);
    
    //-------------------------------------------------------------------------
    bool should_ban() const
    {
        LOCK(cs_);
        return should_ban_;
    }
    
    void set_should_ban(bool should_ban)
    {
        LOCK(cs_);
        should_ban_ = should_ban;
    }
    
    int score() const
    {
        LOCK(cs_);
        return score_;
    }
    
    void set_score(int score)
    {
        LOCK(cs_);
        score_ = score;
    }
    
    int unconnection_headers_len() const
    {
        LOCK(cs_);
        return unconnecting_headers_len_;
    }
    
    void set_unconnection_headers_len(int unconnecting_headers_len)
    {
        LOCK(cs_);
        unconnecting_headers_len_ = unconnecting_headers_len;
    }
    
private:
    mutable util::CriticalSection cs_;
    
    //! Whether this peer should be disconnected and banned (unless whitelisted).
    bool should_ban_ = false;  
    
    //! Accumulated misbehaviour score for this peer.
    int score_ = 0;
    
    //! Length of current-streak of unconnecting headers announcements
    int unconnecting_headers_len_ = 0; 
};

class BlockSyncTimeout {
public:
    int64_t timeout() const
    {
        LOCK(cs_);
        return timeout_;
    }
    
    void set_timeout(int64_t timeout)
    {
        LOCK(cs_);
        timeout_ = timeout;
    }
    
    const chain::BlockIndex *work_header() const
    {
        LOCK(cs_);
        return work_header_;
    }
    
    void set_work_header(const chain::BlockIndex *const work_header)
    {
        LOCK(cs_);
        work_header_ = work_header;
    }
    
    bool sent_getheaders() const
    {
        LOCK(cs_);
        return sent_getheaders_;
    }
    
    void set_sent_getheaders(bool sent_getheaders)
    {
        LOCK(cs_);
        sent_getheaders_ = sent_getheaders;
    }
    
    bool protect() const
    {
        LOCK(cs_);
        return protect_;
    }
    
    void set_protect(bool protect)
    {
        LOCK(cs_);
        protect_ = protect;
    }
    
private:
    mutable util::CriticalSection cs_;
    
    // A timeout used for checking whether our peer has sufficiently synced
    int64_t timeout_ = 0;
    
    // A header with the work we require on our peer's chain
    const chain::BlockIndex *work_header_ = nullptr;
    
    // After timeout is reached, set to true after sending getheaders
    bool sent_getheaders_ = false;
    
    // Whether this peer is protected from disconnection due to a bad/slow chain
    bool protect_ = false;
};

class BlockSyncState1 {
public:
    bool sync_started() const
    {
        return sync_started_;
    }
    
    void set_sync_started(bool sync_started)
    {
        sync_started_ = sync_started;
    }
    
    const chain::BlockIndex *best_known_block_index() const
    {
        return best_known_block_index_;
    }
    
    void set_best_known_block_index(const chain::BlockIndex* const index)
    {
        best_known_block_index_ = index;
    }
    
    const chain::BlockIndex *last_common_block_index() const
    {
        return last_common_block_index_;
    }
    
    void set_last_common_block_index(const chain::BlockIndex* const index)
    {
        last_common_block_index_ = index;
    }
    
    const chain::BlockIndex *best_header_sent_index() const
    {
        return best_header_sent_index_;
    }
    
    void set_best_header_sent_index(const chain::BlockIndex* const index)
    {
        best_header_sent_index_ = index;
    }
    
    util::Hash256 last_unknown_block_hash() const
    {
        LOCK(cs_lash_unknown_);
        return last_unknown_block_hash_;
    }
    
    void set_last_unknown_block_hash(util::Hash256 hash)
    {
        LOCK(cs_lash_unknown_);
        last_unknown_block_hash_ = hash;
    }
    
private:
    //! Whether we've started headers synchronization with this peer.
    std::atomic<bool> sync_started_ = false;
    
    //! The best known block we know this peer has announced.
    std::atomic<const chain::BlockIndex*> best_known_block_index_ = nullptr;
    
    //! The last full block we both have.
    std::atomic<const chain::BlockIndex*> last_common_block_index_ = nullptr;
    
    //! The best header we have sent our peer.
    std::atomic<const chain::BlockIndex*> best_header_sent_index_ = nullptr;
    
    //! The hash of the last unknown block this peer has announced.
    mutable util::CriticalSection cs_lash_unknown_;
    util::Hash256 last_unknown_block_hash_;
};

class NodeBlocksInFlight {
public:
    using iterator = std::list<QueuedBlock>::iterator;
    
    std::list<QueuedBlock> list() const // thread safe copy
    {
        LOCK(cs_);
        return list_;
    }
    
    int valid_headers_count() const
    {
        LOCK(cs_);
        return valid_headers_count_;
    }
    
    void set_valid_headers_count(int valid_headers_count)
    {
        LOCK(cs_);
        valid_headers_count_ = valid_headers_count;
    }
    
private:
    mutable util::CriticalSection cs_;
    std::list<QueuedBlock> list_;
    
    //! When the first entry in BlocksInFlight started downloading. Don't care when BlocksInFlight is empty.
    int64_t downloading_since_ = 0;
    
    int valid_headers_count_ = 0;
};

struct RelayState {    
    //! Whether this peer wants invs or headers (when possible) for block announcements.
    std::atomic<bool> prefer_headers = false;
    
    //! Whether this peer wants invs or cmpctblocks (when possible) for block announcements.
    std::atomic<bool> prefer_header_and_ids = false;
    
    /*
      * Whether this peer will send us cmpctblocks if we request them.
      * This is not used to gate request logic, as we really only care about supports_desired_cmpct_version_,
      * but is used as a flag to "lock in" the version of compact blocks (fWantsCmpctWitness) we send.
      */
    std::atomic<bool> provides_header_and_ids = false;
    
    //! Whether this peer can give us witnesses
    std::atomic<bool> is_witness = false;
    
    //! Whether this peer wants witnesses in cmpctblocks/blocktxns
    std::atomic<bool> wants_cmpct_witness = false;
    
    /*
     * If we've announced kNodeWitness to this peer: whether the peer sends witnesses in cmpctblocks/blocktxns,
     * otherwise: whether this peer sends non-witnesses in cmpctblocks/blocktxns.
     */
    std::atomic<bool> supports_desired_cmpct_version = false;
};

/* Information about a connected peer */
class Node : util::Uncopyable, public std::enable_shared_from_this<Node> {
public:
    Node(const struct bufferevent *bev, const NetAddr& addr,
         bool is_inbound = true, bool manual = false, 
         std::string host_name = "");
    
    //-------------------------------------------------------------------------
    void StopAllTimers();
    bool CheckBanned();
    
    bool ShouldUpdateTime()
    {
        if (misbehavior_.score() == 0 && 
                connection_.connection_state() == NodeConnection::kEstablished)
            return true;
        return false;
    }
    
    bool IsPreferedDownload() const
    {
        return (!connection_.is_inbound() && !protocol_.IsClient());
    }
    
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
    
    const FloodingAddrs& flooding_addrs() const
    {
        return flooding_addrs_;
    }
    
    FloodingAddrs *mutable_flooding_addrs()
    {
        return &flooding_addrs_;
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
    
    const Misbehavior& misbehavior() const
    {
        return misbehavior_;
    }
    
    Misbehavior *mutable_misbehavior()
    {
        return &misbehavior_;
    }
    
    const BlockSyncTimeout& block_sync_timeout() const
    {
        return block_sync_timeout_;
    }
    
    BlockSyncTimeout *mutable_block_sync_timeout()
    {
        return &block_sync_timeout_;
    }
    
    const BlockSyncState1& block_sync_state() const
    {
        return block_sync_state_;
    }
    
    BlockSyncState1 *mutable_block_sync_state()
    {
        return &block_sync_state_;
    }
    
    const NodeBlocksInFlight& blocks_in_flight() const
    {
        return blocks_in_flight_;
    }
    
    NodeBlocksInFlight *mutable_blocks_in_flight()
    {
        return &blocks_in_flight_;
    }
    
    const RelayState& relay_state() const
    {
        return relay_state_;
    }
    
    RelayState *mutable_relay_state()
    {
        return &relay_state_;
    }
     
private:
    const NodeId id_;
    const uint64_t local_host_nonce_;  
    NodeProtocol protocol_;
    NodeConnection connection_;
    //const uint64_t keyed_net_group_;      
    FloodingAddrs flooding_addrs_; // flooding addrs that need to relay    
    NodeFilter filter_;    
    NodeTime time_;    
    NodeTimers timers_;
    Misbehavior misbehavior_;
    BlockSyncTimeout block_sync_timeout_;
    BlockSyncState1 block_sync_state_;
    NodeBlocksInFlight blocks_in_flight_;
    RelayState relay_state_;
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
    void GetNode(const SubNet& subnet, std::vector<std::shared_ptr<Node> > *out);
    void EraseNode(std::shared_ptr<Node> node);    
    void EraseNode(NodeId id);
    
    //-------------------------------------------------------------------------
    void Clear()
    {
        LOCK(cs_nodes_);
        list_.clear();
    }
    
    //-------------------------------------------------------------------------
    //bool AttemptToEvictConnection();
    int CountInbound();
    int CountOutbound();
    int CountSyncStarted();
    int CountPreferredDownload();
    int CountValidatedDownload();
    int CountProtectedOutbound();
    bool ShouldDnsLookup();
    bool CheckIncomingNonce(uint64_t nonce);
    
private:
    mutable util::CriticalSection cs_nodes_;
    std::list<std::shared_ptr<Node> > list_;
    
    //void MakeEvictionCandidate(std::vector<NodeEvictionCandidate> *out);
};

class BlocksInFlight1 {
public:
    using MapBlocksInFlight = 
        std::map<util::Hash256, std::pair<NodeId, NodeBlocksInFlight::iterator> >;
    
    void Erase(const util::Hash256& hash)
    {
        LOCK(cs_);
        map_blocks_in_flight_.erase(hash);
    }
    
private:
    mutable util::CriticalSection cs_;
    MapBlocksInFlight map_blocks_in_flight_;    
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

class SingletonBlocksInFlight : util::Uncopyable {
public:
    static BlocksInFlight1& GetInstance()
    {
        static BlocksInFlight1 blocks_in_flight;
        return blocks_in_flight;
    }
    
private:
    SingletonBlocksInFlight() {}
};


void DisconnectNode(std::shared_ptr<Node> node);
void DisconnectNode(const SubNet& subnet);

namespace NodeTimeoutCb {

void InactivityTimeoutCb(std::shared_ptr<Node> node);
void SocketNoMsgTimeoutCb(std::shared_ptr<Node> node);
void PingTimeoutCb(std::shared_ptr<Node> node, uint32_t magic);
void ShakeHandsTimeoutCb(std::shared_ptr<Node> node);

} // namespace NodeTimeoutCb


} // namespace network
} // namespace btclite


#endif // BTCLITE_NODE_H
