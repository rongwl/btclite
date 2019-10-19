#ifndef BTCLITE_BLOCK_SYNC_H
#define BTCLITE_BLOCK_SYNC_H


#include <list>

#include "chain.h"
#include "network_address.h"


using NodeId = int64_t;

class BlockReject {
public:
    uint8_t reject_code() const
    {
        LOCK(cs_);
        return rejec_code_;
    }
    void set_reject_code(uint8_t code)
    {
        LOCK(cs_);
        rejec_code_ = code;
    }
    
    std::string reject_reason() const // thread safe copy
    {
        LOCK(cs_);
        return reject_reason_;
    }
    void set_reject_reason(const std::string& reason)
    {
        LOCK(cs_);
        reject_reason_ = reason;
    }
    void set_reject_reason(std::string&& reason) noexcept
    {
        LOCK(cs_);
        reject_reason_ = std::move(reason);
    }
    
    Hash256 block_hash() const // thread safe copy
    {
        LOCK(cs_);
        return block_hash_;
    }
    void set_block_hash(const Hash256& hash)
    {
        LOCK(cs_);
        block_hash_ = hash;
    }
    void set_block_hash(Hash256&& hash) noexcept
    {
        LOCK(cs_);
        block_hash_ = std::move(hash);
    }

private:
    mutable CriticalSection cs_;
    uint8_t rejec_code_ = 0;
    std::string reject_reason_;
    Hash256 block_hash_;
};

/* Blocks that are in flight, and that are in the queue to be downloaded. */
class QueuedBlock {
public:
    Hash256 hash() const // thread safe copy
    {
        LOCK(cs_);
        return hash_;
    }
    void set_hash(const Hash256& hash)
    {
        LOCK(cs_);
        hash_ = hash;
    }
    void set_hash(Hash256&& hash)
    {
        LOCK(cs_);
        hash_ = std::move(hash);
    }
    
    const BlockIndex *index() const
    {
        LOCK(cs_);
        return index_;
    }
    void set_index(const BlockIndex *index)
    {
        LOCK(cs_);
        index_ = index;
    }
    
    bool has_validated_headers() const
    {
        LOCK(cs_);
        return has_validated_headers_;
    }
    void set_has_validated_headers(bool has_validated_headers) 
    {
        LOCK(cs_);
        has_validated_headers_ = has_validated_headers;
    }
    
private:
    mutable CriticalSection cs_;
    Hash256 hash_;
    const BlockIndex* index_ = nullptr;
    
    // Whether this block has validated headers at the time of request.
    bool has_validated_headers_;
    
    // Optional, used for CMPCTBLOCK downloads
    //std::unique_ptr<PartiallyDownloadedBlock> partialBlock;
};

class SyncBasicState {
public:
    bool connected() const
    {
        LOCK(cs_);
        return connected_;
    }
    void set_connected(bool connected)
    {
        LOCK(cs_);
        connected_ = connected;
    }
    
    bool sync_started() const
    {
        LOCK(cs_);
        return sync_started_;
    }
    void set_sync_started(bool sync_started)
    {
        LOCK(cs_);
        sync_started_ = sync_started;
    }
    
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
    
private:
    mutable CriticalSection cs_;
    
    //! Whether we have a fully established connection.
    bool connected_ = false;
    
    //! Whether we've started headers synchronization with this peer.
    bool sync_started_ = false;
    
    //! Whether this peer should be disconnected and banned (unless whitelisted).
    bool should_ban_ = false;  
};

class SyncStats {
public:
    int misbehavior_score() const
    {
        LOCK(cs_);
        return misbehavior_score_;
    }
    void set_misbehavior_score(int misbehavior_score)
    {
        LOCK(cs_);
        misbehavior_score_ = misbehavior_score;
    }
    
    int unconnecting_headers_len() const
    {
        LOCK(cs_);
        return unconnecting_headers_len_;
    }
    void set_unconnecting_headers_len(int unconnecting_headers_len)
    {
        LOCK(cs_);
        unconnecting_headers_len_ = unconnecting_headers_len;
    }
    
    const std::vector<BlockReject>& rejects() const
    {
        return rejects_;
    }
    std::vector<BlockReject> *mutable_rejects()
    {
        return &rejects_;
    }

private:
    mutable CriticalSection cs_;
    
    //! Accumulated misbehaviour score for this peer.
    int misbehavior_score_ = 0;
    
    //! Length of current-streak of unconnecting headers announcements
    int unconnecting_headers_len_ = 0; 
    
    //! List of asynchronously-determined block rejections to notify this peer about.
    std::vector<BlockReject> rejects_;
};

class SyncTimeoutState {
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
    
    const BlockIndex *work_header() const
    {
        LOCK(cs_);
        return work_header_;
    }
    void set_work_header(const BlockIndex *index)
    {
        LOCK(cs_);
        work_header_ = index;
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
    mutable CriticalSection cs_;
    
    // A timeout used for checking whether our peer has sufficiently synced
    int64_t timeout_ = 0;
    
    // A header with the work we require on our peer's chain
    const BlockIndex *work_header_ = nullptr;
    
    // After timeout is reached, set to true after sending getheaders
    bool sent_getheaders_ = false;
    
    // Whether this peer is protected from disconnection due to a bad/slow chain
    bool protect_ = false;
};

class SyncTimeState {
public:
    int64_t headers_sync_timeout() const
    {
        LOCK(cs_);
        return headers_sync_timeout_;
    }
    void set_headers_sync_timeout(int64_t headers_sync_timeout)
    {
        LOCK(cs_);
        headers_sync_timeout_ = headers_sync_timeout;
    }
    
    int64_t stalling_since() const
    {
        LOCK(cs_);
        return stalling_since_;
    }
    void set_stalling_since(int64_t stalling_since)
    {
        LOCK(cs_);
        stalling_since_ = stalling_since;
    }
    
    int64_t last_block_announcement() const
    {
        LOCK(cs_);
        return last_block_announcement_;
    }
    void set_last_block_announcement(int64_t last_block_announcement)
    {
        LOCK(cs_);
        last_block_announcement_ = last_block_announcement;
    }
    
    const SyncTimeoutState& timeout_state() const
    {
        LOCK(cs_);
        return timeout_state_;
    }
    SyncTimeoutState *mutable_timeout_state()
    {
        return &timeout_state_;
    }
    
private:
    mutable CriticalSection cs_;
    
    //! When to potentially disconnect peer for stalling headers download
    int64_t headers_sync_timeout_ = 0;
    
    //! Since when we're stalling block download progress (in microseconds), or 0.
    int64_t stalling_since_ = 0;
    
    //! Time of last new block announcement
    int64_t last_block_announcement_ = 0;
    
    SyncTimeoutState timeout_state_;
};

class SyncProcessState {
public:
    const BlockIndex *best_known_block_index() const
    {
        LOCK(cs_);
        return best_known_block_index_;
    }
    void set_best_known_block_index(const BlockIndex *index)
    {
        LOCK(cs_);
        best_known_block_index_ = index;
    }
    
    const BlockIndex *last_common_block_index() const
    {
        LOCK(cs_);
        return last_common_block_index_;
    }
    void set_last_common_block_index(const BlockIndex *index)
    {
        LOCK(cs_);
        last_common_block_index_ = index;
    }
    
    const BlockIndex *best_header_sent_index() const
    {
        LOCK(cs_);
        return best_header_sent_index_;
    }
    void set_best_header_sent_index(const BlockIndex *index)
    {
        LOCK(cs_);
        best_header_sent_index_ = index;
    }
    
    Hash256 last_unknown_block_hash() const // thread safe copy
    {
        LOCK(cs_);
        return last_unknown_block_hash_;
    }
    void set_last_unknown_block_hash(const Hash256& last_unknown_block_hash)
    {
        LOCK(cs_);
        last_unknown_block_hash_ = last_unknown_block_hash;
    }
    void set_last_unknown_block_hash(Hash256&& last_unknown_block_hash)
    {
        LOCK(cs_);
        last_unknown_block_hash_ = std::move(last_unknown_block_hash);
    }
    
private:
    mutable CriticalSection cs_;
    
    //! The best known block we know this peer has announced.
    const BlockIndex *best_known_block_index_ = nullptr;
    
    //! The last full block we both have.
    const BlockIndex *last_common_block_index_ = nullptr;
    
    //! The best header we have sent our peer.
    const BlockIndex *best_header_sent_index_ = nullptr;
    
    //! The hash of the last unknown block this peer has announced.
    Hash256 last_unknown_block_hash_;
};

class BlocksInFlight {
public:
    using iterator = std::list<QueuedBlock>::iterator;
    
    const std::list<QueuedBlock>& list() const
    {
        return list_;
    }
    std::list<QueuedBlock> *mutable_list()
    {
        return &list_;
    }
    
    int64_t downloading_since() const
    {
        LOCK(cs_);
        return downloading_since_;
    }
    void set_downloading_since(int64_t downloading_since)
    {
        LOCK(cs_);
        downloading_since_ = downloading_since;
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
    mutable CriticalSection cs_;
    
    std::list<QueuedBlock> list_;
    
    //! When the first entry in BlocksInFlight started downloading. Don't care when BlocksInFlight is empty.
    int64_t downloading_since_ = 0;
    
    //int count_;
    
    int valid_headers_count_ = 0;
};

class DownloadState {
public:
    bool preferred_download() const
    {
        LOCK(cs_);
        return preferred_download_;
    }
    void set_preferred_download(bool preferred_download)
    {
        LOCK(cs_);
        preferred_download_ = preferred_download;
    }
    
    bool prefer_headers() const
    {
        LOCK(cs_);
        return prefer_headers_;
    }
    void set_prefer_headers(bool prefer_headers)
    {
        LOCK(cs_);
        prefer_headers_ = prefer_headers;
    }
    
    bool prefer_header_and_ids() const
    {
        LOCK(cs_);
        return prefer_header_and_ids_;
    }
    void set_prefer_header_and_ids(bool prefer_header_and_ids)
    {
        LOCK(cs_);
        prefer_header_and_ids_ = prefer_header_and_ids;
    }
    
    bool provides_header_and_ids() const
    {
        LOCK(cs_);
        return provides_header_and_ids_;
    }
    void set_provides_header_and_ids(bool provides_header_and_ids)
    {
        LOCK(cs_);
        provides_header_and_ids_ = provides_header_and_ids;
    }
    
    bool is_witness() const
    {
        LOCK(cs_);
        return is_witness_;
    }
    void set_is_witness(bool is_witness)
    {
        LOCK(cs_);
        is_witness_ = is_witness;
    }
    
    bool wants_cmpct_witness() const
    {
        LOCK(cs_);
        return wants_cmpct_witness_;
    }
    void set_wants_cmpct_witness(bool wants_cmpct_witness)
    {
        LOCK(cs_);
        wants_cmpct_witness_ = wants_cmpct_witness;
    }
    
    bool supports_desired_cmpct_version() const
    {
        LOCK(cs_);
        return supports_desired_cmpct_version_;
    }
    void set_supports_desired_cmpct_version(bool supports_desired_cmpct_version)
    {
        LOCK(cs_);
        supports_desired_cmpct_version_ = supports_desired_cmpct_version;
    }
    
private:
    mutable CriticalSection cs_;
    
    //! Whether we consider this a preferred download peer.
    bool preferred_download_ = false;
    
    //! Whether this peer wants invs or headers (when possible) for block announcements.
    bool prefer_headers_ = false;
    
    //! Whether this peer wants invs or cmpctblocks (when possible) for block announcements.
    bool prefer_header_and_ids_ = false;
    
    /*
      * Whether this peer will send us cmpctblocks if we request them.
      * This is not used to gate request logic, as we really only care about supports_desired_cmpct_version_,
      * but is used as a flag to "lock in" the version of compact blocks (fWantsCmpctWitness) we send.
      */
    bool provides_header_and_ids_ = false;
    
    //! Whether this peer can give us witnesses
    bool is_witness_ = false;
    
    //! Whether this peer wants witnesses in cmpctblocks/blocktxns
    bool wants_cmpct_witness_ = false;
    
    /*
     * If we've announced kNodeWitness to this peer: whether the peer sends witnesses in cmpctblocks/blocktxns,
     * otherwise: whether this peer sends non-witnesses in cmpctblocks/blocktxns.
     */
    bool supports_desired_cmpct_version_ = false;
};

// Maintain validation-specific state about block sync
class BlockSyncState {
public:
    BlockSyncState(const btclite::network::NetAddr& addr, const std::string& addr_name)
        : node_addr_(addr), node_name_(addr_name) {}
    
    const btclite::network::NetAddr& node_addr() const
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
    const btclite::network::NetAddr node_addr_;    
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
    
    void AddSyncState(NodeId id, const btclite::network::NetAddr& addr, const std::string& addr_name);    
    BlockSyncState *GetSyncState(NodeId id);
    void EraseSyncState(NodeId id);
    bool ShouldUpdateTime(NodeId id);
    void Misbehaving(NodeId id, int howmuch);
    bool CheckBanned(NodeId id);    

private:
    mutable CriticalSection cs_block_sync_;
    
    // Number of nodes with SyncBasicState::sync_started.
    int sync_started_count_ = 0;
    
    // Number of preferable block download peers.
    int preferred_download_count_ = 0;
    
    // Number of peers from which we're downloading blocks.
    int validated_download_count_ = 0;

    // Number of outbound peers with SyncTimeoutState::protect.
    int protected_outbound_count_ = 0;
    
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
