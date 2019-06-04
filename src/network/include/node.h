#ifndef BTCLITE_NODE_H
#define BTCLITE_NODE_H


#include <list>

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
    NodeState(btclite::NetAddr& addr, std::string addr_name);
    
private:
    mutable CriticalSection cs_node_state_;
    
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
    using NodeId = int64_t;
    
    Node()
        : id_(0), socket_(), disconnected_(false) {}
    
    //-------------------------------------------------------------------------
    void Connect();
    void Disconnect();
    size_t Receive();
    size_t Send();
    
    //-------------------------------------------------------------------------
    NodeId id() const
    {
        return id_;
    }
    
    const BasicSocket& socket() const
    {
        return socket_;
    }
    
    BasicSocket* mutable_socket()
    {
        return &socket_;
    }
    
    bool disconnected() const
    {
        return disconnected_;
    }
    
    void set_disconnected(bool disconnected)
    {
        disconnected_ = disconnected;
    }
    
private:
    NodeId id_;
    BasicSocket socket_;
    std::atomic_bool disconnected_;
    SemaphoreGrant grant_outbound_;
};

class Nodes {
public:
    Nodes()
        : list_() {}
    
    void ClearDisconnected();
    void CheckInactive();
    
    const std::list<Node>& list() const
    {
        return list_;
    }
    
private:
    mutable CriticalSection cs_nodes_;
    std::list<Node> list_;
    uint32_t n_sync_started_;
    
    void ClearNodeState();
};

#endif // BTCLITE_NODE_H
