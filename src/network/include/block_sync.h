#ifndef BTCLITE_BLOCK_SYNC_H
#define BTCLITE_BLOCK_SYNC_H


#include <list>

#include "chain.h"
#include "net.h"


struct BlockReject {
    BlockReject()
        : rejec_code(0), reject_reason(), block_hash() {}
    
    unsigned char rejec_code;
    std::string reject_reason;
    Hash256 block_hash;
};

/* Blocks that are in flight, and that are in the queue to be downloaded. */
struct QueuedBlock {
    QueuedBlock()
        : hash(), index(nullptr), validated_headers(false) {}
    
    Hash256 hash;
    const BlockIndex* index;
    
    // Whether this block has validated headers at the time of request.
    bool validated_headers;
    
    // Optional, used for CMPCTBLOCK downloads
    //std::unique_ptr<PartiallyDownloadedBlock> partialBlock;
};

struct SyncBasicState {
    SyncBasicState()
        : connected(false), sync_started(false), should_ban(false) {}
    
    //! Whether we have a fully established connection.
    bool connected;
    
    //! Whether we've started headers synchronization with this peer.
    bool sync_started;
    
    //! Whether this peer should be disconnected and banned (unless whitelisted).
    bool should_ban;  
};

struct SyncStats {
    SyncStats()
        : misbehavior_score(0), unconnecting_headers_len(0), rejects() {}
    
    //! Accumulated misbehaviour score for this peer.
    int misbehavior_score;
    
    //! Length of current-streak of unconnecting headers announcements
    int unconnecting_headers_len; 
    
    //! List of asynchronously-determined block rejections to notify this peer about.
    std::vector<BlockReject> rejects;
};

struct SyncTimeoutState {
    SyncTimeoutState()
        : timeout(0), work_header(nullptr), 
          sent_getheaders(false), protect(false) {}
    
    // A timeout used for checking whether our peer has sufficiently synced
    int64_t timeout;
    
    // A header with the work we require on our peer's chain
    const BlockIndex *work_header;
    
    // After timeout is reached, set to true after sending getheaders
    bool sent_getheaders;
    
    // Whether this peer is protected from disconnection due to a bad/slow chain
    bool protect;
};

struct SyncTimeState {
    SyncTimeState()
        : headers_sync_timeout(0), stalling_since(0),
          last_block_announcement(0), timeout_state() {}
    
    //! When to potentially disconnect peer for stalling headers download
    int64_t headers_sync_timeout;
    
    //! Since when we're stalling block download progress (in microseconds), or 0.
    int64_t stalling_since;
    
    //! Time of last new block announcement
    int64_t last_block_announcement;
    
    SyncTimeoutState timeout_state;
};

struct SyncProcessState {
    SyncProcessState()
        : best_known_block_index(nullptr), last_common_block_index(nullptr),
          best_header_sent_index(nullptr), last_unknown_block_hash() {}
    
    //! The best known block we know this peer has announced.
    const BlockIndex *best_known_block_index;
    
    //! The last full block we both have.
    const BlockIndex *last_common_block_index;
    
    //! The best header we have sent our peer.
    const BlockIndex *best_header_sent_index;
    
    //! The hash of the last unknown block this peer has announced.
    Hash256 last_unknown_block_hash;
};

struct BlocksInFlight {
    using iterator = std::list<QueuedBlock>::iterator;
    
    BlocksInFlight()
        : list(), downloading_since(0), valid_headers_count(0) {}
    
    std::list<QueuedBlock> list;
    
    //! When the first entry in vBlocksInFlight started downloading. Don't care when vBlocksInFlight is empty.
    int64_t downloading_since;
    
    //int count_;
    
    int valid_headers_count;
};

struct DownloadState {
    DownloadState()
        : preferred_download(false), prefer_headers(false),
          prefer_header_and_ids(false), provides_header_and_ids(false), 
          is_witness(false), wants_cmpct_witness(false),
          supports_desired_cmpct_version(false) {}
    
    //! Whether we consider this a preferred download peer.
    bool preferred_download;
    
    //! Whether this peer wants invs or headers (when possible) for block announcements.
    bool prefer_headers;
    
    //! Whether this peer wants invs or cmpctblocks (when possible) for block announcements.
    bool prefer_header_and_ids;
    
    /*
      * Whether this peer will send us cmpctblocks if we request them.
      * This is not used to gate request logic, as we really only care about supports_desired_cmpct_version_,
      * but is used as a flag to "lock in" the version of compact blocks (fWantsCmpctWitness) we send.
      */
    bool provides_header_and_ids;
    
    //! Whether this peer can give us witnesses
    bool is_witness;
    
    //! Whether this peer wants witnesses in cmpctblocks/blocktxns
    bool wants_cmpct_witness;
    
    /*
     * If we've announced NODE_WITNESS to this peer: whether the peer sends witnesses in cmpctblocks/blocktxns,
     * otherwise: whether this peer sends non-witnesses in cmpctblocks/blocktxns.
     */
    bool supports_desired_cmpct_version;
};

// Maintain validation-specific state about block sync
class BlockSyncState {
public:
    BlockSyncState(const btclite::NetAddr& addr, const std::string& addr_name)
        : node_addr_(addr), node_name_(addr_name) {}
    
    const btclite::NetAddr& node_addr() const
    {
        return node_addr_;
    }
    
    const SyncBasicState& basic_state() const
    {
        return basic_state_;
    }
    
    SyncBasicState *mutable_basic_state()
    {
        return &basic_state_;
    }
    
    const SyncStats& stats() const
    {
        return stats_;
    }
    
    SyncStats *mutable_stats()
    {
        return &stats_;
    }
    
    const SyncTimeState& time_state() const
    {
        return time_state_;
    }
    
    SyncTimeState *mutable_time_state()
    {
        return &time_state_;
    }
    
    const SyncProcessState& process_state() const
    {
        return process_state_;
    }
    
    SyncProcessState *mutable_process_state()
    {
        return &process_state_;
    }
    
    const BlocksInFlight& blocks_in_flight() const
    {
        return blocks_in_flight_;
    }
    
    BlocksInFlight *mutable_blocks_inflight()
    {
        return &blocks_in_flight_;
    }
    
    const DownloadState& download_state() const
    {
        return download_state_;
    }
    
    DownloadState *mutable_download_state()
    {
        return &download_state_;
    }
    
private:    
    const btclite::NetAddr node_addr_;    
    const std::string node_name_;    
    SyncBasicState basic_state_;    
    SyncStats stats_;    
    SyncTimeState time_state_;    
    SyncProcessState process_state_;    
    BlocksInFlight blocks_in_flight_;    
    DownloadState download_state_;   
};

class BlockSync : Uncopyable {
public:
    using MapPeerSyncState = std::map<NodeId, BlockSyncState>;
    using MapBlockInFlight = std::map<Hash256, std::pair<NodeId, BlocksInFlight::iterator> >;
    
    void AddSyncState(NodeId id, const btclite::NetAddr& addr, const std::string& addr_name)
    {
        LOCK(cs_block_sync_);
        map_sync_state_.emplace_hint(map_sync_state_.end(), std::piecewise_construct,
                                     std::forward_as_tuple(id), std::forward_as_tuple(addr, std::move(addr_name)));
    }
    
    // not thread safe, just for unit test
    const BlockSyncState* const GetSyncState(NodeId id) const;    
    void EraseSyncState(NodeId id);
    bool ShouldUpdateTime(NodeId id);

private:
    mutable CriticalSection cs_block_sync_;
    
    // Number of nodes with SyncBasicState::sync_started.
    int sync_started_count_;
    
    // Number of preferable block download peers.
    int preferred_download_count_;
    
    // Number of peers from which we're downloading blocks.
    int validated_download_count_;

    // Number of outbound peers with SyncTimeoutState::protect.
    int protected_outbound_count_;
    
    MapPeerSyncState map_sync_state_;
    MapBlockInFlight map_block_in_flight_;
};

class SingletonBlockSync : Uncopyable {
public:
    static BlockSync& GetInstance()
    {
        static BlockSync block_sync;
        return block_sync;
    }
    
private:
    SingletonBlockSync() {}
};

template <typename Iter>
struct IterLess
{
    bool operator()(const Iter& a, const Iter& b) const
    {
        return &(*a) < &(*b);
    }
};

struct OrphanTx {
    using TxSharedPtr = std::shared_ptr<const Transaction>;
    
    // When modifying, adapt the copy of this definition in unit tests.
    TxSharedPtr tx;
    NodeId from_peer;
    int64_t time_expire;
};

class Orphans : Uncopyable {
public:
    using MapOrphanTxs = std::map<Hash256, OrphanTx>;
    using MapPrevOrphanTxs = std::map<OutPoint, std::set<MapOrphanTxs::iterator, IterLess<MapOrphanTxs::iterator> > >;
    
    bool AddOrphanTx(OrphanTx::TxSharedPtr tx, NodeId id);
    void EraseOrphansFor(NodeId id);
    uint32_t LimitOrphanTxSize(uint32_t max_orphans);
    
private:
    mutable CriticalSection cs_orphans_;
    MapOrphanTxs map_orphan_txs_;
    MapPrevOrphanTxs map_prev_orphan_txs_;
    
    int EraseOrphanTx(const Hash256& hash);
};

class SingletonOrphans : Uncopyable {
public:
    static Orphans& GetInstance()
    {
        static Orphans orphans;
        return orphans;
    }
    
private:
    SingletonOrphans() {}
};


#endif // BTCLITE_BLOCK_SYNC_H
