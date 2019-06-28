#ifndef BTCLITE_NODE_H
#define BTCLITE_NODE_H


#include <list>

#include "bloom.h"
#include "chain.h"
#include "hash.h"
#include "socket.h"


struct ChainSyncTimeoutState {
    // A timeout used for checking whether our peer has sufficiently synced
    int64_t timeout_;
    
    // A header with the work we require on our peer's chain
    const BlockIndex *work_header_;
    
    // After timeout is reached, set to true after sending getheaders
    bool sent_getheaders_;
    
    // Whether this peer is protected from disconnection due to a bad/slow chain
    bool protect_;
};

struct BlockReject {
    unsigned char rejec_code_;
    std::string reject_reason_;
    Hash256 block_hash_;
};

/* Blocks that are in flight, and that are in the queue to be downloaded. */
struct QueuedBlock {
    Hash256 hash_;
    const BlockIndex* index_;
    
    // Whether this block has validated headers at the time of request.
    bool validated_headers_;
    
    // Optional, used for CMPCTBLOCK downloads
    //std::unique_ptr<PartiallyDownloadedBlock> partialBlock;
};

// Maintain validation-specific state about nodes
class NodeState {
public:
    NodeState(const btclite::NetAddr& addr, const std::string& addr_name);
    
private:
    //! The peer's address
    const btclite::NetAddr address_;
    
    //! Whether we have a fully established connection.
    bool connected_;
    
    //! Accumulated misbehaviour score for this peer.
    int misbehavior_score_;
    
    //! Whether this peer should be disconnected and banned (unless whitelisted).
    bool should_ban_;
    
    //! String name of this peer (debugging/logging purposes).
    const std::string name_;
    
    //! List of asynchronously-determined block rejections to notify this peer about.
    std::vector<BlockReject> rejects_;
    
    //! The best known block we know this peer has announced.
    const BlockIndex *best_known_block_index_;
    
    //! The hash of the last unknown block this peer has announced.
    Hash256 last_unknown_block_hash_;
    
    //! The last full block we both have.
    const BlockIndex *last_common_block_index_;
    
    //! The best header we have sent our peer.
    const BlockIndex *best_header_sent_index_;
    
    //! Length of current-streak of unconnecting headers announcements
    int unconnecting_headers_len_;
    
    //! Whether we've started headers synchronization with this peer.
    bool sync_started_;
    
    //! When to potentially disconnect peer for stalling headers download
    int64_t headers_sync_timeout_;
    
    //! Since when we're stalling block download progress (in microseconds), or 0.
    int64_t stalling_since_;
    
    std::list<QueuedBlock> block_list_in_flight_;
    
    //! When the first entry in vBlocksInFlight started downloading. Don't care when vBlocksInFlight is empty.
    int64_t downloading_since_;
    
    int blocks_in_flight_;
    
    int blocks_in_flight_valid_headers_;
    
    //! Whether we consider this a preferred download peer.
    bool preferred_download_;
    
    //! Whether this peer wants invs or headers (when possible) for block announcements.
    bool prefer_headers_;
    
    //! Whether this peer wants invs or cmpctblocks (when possible) for block announcements.
    bool prefer_header_and_ids_;
    
    /*
      * Whether this peer will send us cmpctblocks if we request them.
      * This is not used to gate request logic, as we really only care about fSupportsDesiredCmpctVersion,
      * but is used as a flag to "lock in" the version of compact blocks (fWantsCmpctWitness) we send.
      */
    bool provides_header_and_ids_;
    
    //! Whether this peer can give us witnesses
    bool is_witness_;
    
    //! Whether this peer wants witnesses in cmpctblocks/blocktxns
    bool wants_cmpct_witness_;
    
    /*
     * If we've announced NODE_WITNESS to this peer: whether the peer sends witnesses in cmpctblocks/blocktxns,
     * otherwise: whether this peer sends non-witnesses in cmpctblocks/blocktxns.
     */
    bool supports_desired_cmpct_version_;
    
    ChainSyncTimeoutState chain_sync_;

    //! Time of last new block announcement
    int64_t last_block_announcement_;
};

/* Information about a connected peer */
class Node {
public:
    using Id = int64_t;
    
    Node(Id id, ServiceFlags services, int start_height, Socket::Fd sock_fd, const btclite::NetAddr& addr,
         uint64_t local_host_nonce, const btclite::NetAddr &addr_bind, const std::string& host_name, bool is_inbound);
    
    //-------------------------------------------------------------------------
    void Connect();
    void Disconnect();
    size_t Receive();
    size_t Send();
    
    //-------------------------------------------------------------------------
    int64_t time_connected() const
    {
        return time_connected_;
    }
    
    Id id() const
    {
        return id_;
    }
    
    ServiceFlags services() const
    {
        return services_;
    }
    
    Socket::Fd sock_fd() const
    {
        return sock_fd_;
    }
    
    const btclite::NetAddr& addr() const
    {
        return addr_;
    }
    
    /*uint64_t keyed_net_group() const
    {
        return keyed_net_group_;
    }*/
    
    bool is_inbound() const
    {
        return is_inbound_;
    }
    
    bool disconnected() const
    {
        return disconnected_;
    }
    
    void set_disconnected(bool disconnected)
    {
        disconnected_ = disconnected;
    }
    
    std::string host_name() const
    {
        LOCK(cs_host_name_);
        return host_name_;
    }
    
    const BloomFilter *bloom_filter() const
    {
        LOCK(cs_bloom_filter_);
        return bloom_filter_.get();
    }
    
    bool relay_txes() const
    {
        LOCK(cs_bloom_filter_);
        return relay_txes_;
    }
    
    int64_t min_ping_usec_time() const
    {
        return min_ping_usec_time_;
    }
    
    int64_t last_block_time() const
    {
        return last_block_time_;
    }
    
    int64_t last_tx_time() const
    {
        return last_tx_time_;
    }
    
private:
    const int64_t time_connected_;
    const Id id_;
    const ServiceFlags services_;
    const int start_height_;
    Socket::Fd sock_fd_;
    const btclite::NetAddr addr_;
    //const uint64_t keyed_net_group_;
    const uint64_t local_host_nonce_;
    const btclite::NetAddr addr_bind_;
    
    mutable CriticalSection cs_host_name_;
    std::string host_name_;
    
    const bool is_inbound_;
    std::atomic_bool disconnected_;
    SemaphoreGrant grant_outbound_;
    
    mutable CriticalSection cs_bloom_filter_;
    std::unique_ptr<BloomFilter> bloom_filter_;
    // We use fRelayTxes for two purposes -
    // a) it allows us to not relay tx invs before receiving the peer's version message
    // b) the peer may tell us in its version message that we should not relay tx invs
    //    unless it loads a bloom filter.
    bool relay_txes_; // protected by cs_bloom_filter_
    
    // Best measured round-trip time.
    std::atomic<int64_t> min_ping_usec_time_;
    
    // Block and TXN accept times
    std::atomic<int64_t> last_block_time_;
    std::atomic<int64_t> last_tx_time_;
};

class MapNodeState {
public:
    using MapType = std::map<Node::Id, NodeState>;
    
    MapNodeState()
        : map_() {}
    
    void Add(Node::Id id, const btclite::NetAddr& addr, const std::string& addr_name)
    {
        LOCK(cs_map_);
        map_.emplace_hint(map_.end(), std::piecewise_construct,
                          std::forward_as_tuple(id), std::forward_as_tuple(addr, std::move(addr_name)));
    }
    
    const MapType& map() const
    {
        LOCK(cs_map_);
        return map_;
    }
    
private:
    mutable CriticalSection cs_map_; 
    MapType map_;
};

// Singleton pattern, thread safe after c++11
class SingletonMapNodeState {
public:
    static MapNodeState& GetInstance()
    {
        static MapNodeState map_node_state;
        return map_node_state;
    }
    
    SingletonMapNodeState(const SingletonMapNodeState&) = delete;
    SingletonMapNodeState& operator=(const SingletonMapNodeState&) = delete;
    
private:
    SingletonMapNodeState() {}
};

struct NodeEvictionCandidate
{
    Node::Id id;
    int64_t time_connected;
    int64_t min_ping_usec_time;
    int64_t last_block_time;
    int64_t last_tx_time;
    bool relevant_services;
    bool relay_txes;
    bool bloom_filter;
    btclite::NetAddr addr;
    uint64_t keyed_net_group;
};

class Nodes : Uncopyable {
public:
    Nodes()
        : list_() {}
    
    Node::Id GetNewNodeId()
    {
        static std::atomic<Node::Id> last_node_id = 0;
        return last_node_id.fetch_add(1, std::memory_order_relaxed);
    }
    
    void ClearDisconnected();
    void CheckInactive();
    
    bool DisconnectNode(Node::Id id);
    void DisconnectBanNode(const SubNet& subnet);
    //bool AttemptToEvictConnection();
    int CountInbound();
    
    const std::list<Node*>& list() const
    {
        return list_;
    }
    
private:
    mutable CriticalSection cs_nodes_;
    std::list<Node*> list_;
    uint32_t n_sync_started_;
    
    void ClearNodeState();
    //void MakeEvictionCandidate(std::vector<NodeEvictionCandidate> *out);
};

// Singleton pattern, thread safe after c++11
class SingletonNodes {
public:
    static Nodes& GetInstance()
    {
        static Nodes nodes;
        return nodes;
    }
    
    SingletonNodes(const SingletonNodes&) = delete;
    SingletonNodes& operator=(const SingletonNodes&) = delete;
    
private:
    SingletonNodes() {}    
};


#endif // BTCLITE_NODE_H
