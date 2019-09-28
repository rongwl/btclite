#include "block_sync.h"
#include "random.h"
#include "utiltime.h"


void BlockSync::AddSyncState(NodeId id, const btclite::network::NetAddr& addr, const std::string& addr_name)
{
    LOCK(cs_block_sync_);
    map_sync_state_.emplace_hint(map_sync_state_.end(), std::piecewise_construct,
                                 std::forward_as_tuple(id), std::forward_as_tuple(addr, std::move(addr_name)));
    BTCLOG(LOG_LEVEL_VERBOSE) << "Added node state, peer:" << id << " addr:" << addr.ToString();
}

const BlockSyncState* const BlockSync::GetSyncState(NodeId id) const
{
    LOCK(cs_block_sync_);
    auto it = map_sync_state_.find(id);
    if (it != map_sync_state_.end())
        return &(it->second);
    return nullptr;
}

void BlockSync::EraseSyncState(NodeId id)
{
    LOCK(cs_block_sync_);
    
    auto it = map_sync_state_.find(id);
    if (it == map_sync_state_.end())
        return;
    BlockSyncState& state = it->second;
    
    if (state.basic_state().sync_started)
        sync_started_count_--;
    
    for (const QueuedBlock& entry : state.blocks_in_flight().list) {
        map_block_in_flight_.erase(entry.hash);
    }    
    SingletonOrphans::GetInstance().EraseOrphansFor(id);
    preferred_download_count_ -= state.download_state().preferred_download ? 1 : 0;
    validated_download_count_ -= (state.blocks_in_flight().valid_headers_count != 0) ? 1 : 0;
    assert(validated_download_count_ >= 0);
    protected_outbound_count_ -= state.time_state().timeout_state.protect ? 1 : 0;
    assert(protected_outbound_count_ >= 0);

    map_sync_state_.erase(id);
    if (map_sync_state_.empty()) {
        // Do a consistency check after the last peer is removed.
        assert(map_block_in_flight_.empty());
        assert(preferred_download_count_ == 0);
        assert(validated_download_count_ == 0);
        assert(protected_outbound_count_ == 0);
    }
    
    BTCLOG(LOG_LEVEL_INFO) << "Cleared node state for peer " << id;
}

bool BlockSync::ShouldUpdateTime(NodeId id)
{
    LOCK(cs_block_sync_);
    auto it = map_sync_state_.find(id);
    if (it != map_sync_state_.end())
        if (it->second.stats().misbehavior_score == 0 &&
            it->second.basic_state().connected)
            return true;
    
    return false;
}

bool Orphans::AddOrphanTx(OrphanTx::TxSharedPtr tx, NodeId id)
{
    LOCK(cs_orphans_);

    const Hash256& hash = tx->Hash();
    if (map_orphan_txs_.count(hash))
        return false;

    // Ignore big transactions, to avoid a
    // send-big-orphans memory exhaustion attack. If a peer has a legitimate
    // large transaction with a missing parent then we assume
    // it will rebroadcast it later, after the parent transaction(s)
    // have been mined or received.
    // 100 orphans, each of which is at most 99,999 bytes big is
    // at most 10 megabytes of orphans and somewhat more byprev index (in the worst case):
    /*unsigned int sz = GetTransactionWeight(*tx);
    if (sz >= MAX_STANDARD_TX_WEIGHT)
    {
        LogPrint(BCLog::MEMPOOL, "ignoring large orphan tx (size: %u, hash: %s)\n", sz, hash.ToString());
        return false;
    }

    auto ret = map_orphan_txs_.emplace(hash, COrphanTx{tx, id, GetTime() + ORPHAN_TX_EXPIRE_TIME});
    assert(ret.second);
    for (const CTxIn& txin : tx->vin) {
        map_prev_orphan_txs_[txin.prevout].insert(ret.first);
    }

    AddToCompactExtraTransactions(tx);

    LogPrint(BCLog::MEMPOOL, "stored orphan tx %s (mapsz %u outsz %u)\n", hash.ToString(),
             map_orphan_txs_.size(), map_prev_orphan_txs_.size());*/
    return true;
}

void Orphans::EraseOrphansFor(NodeId id)
{
    LOCK(cs_orphans_);
    int erased_count = 0;
    auto it = map_orphan_txs_.begin();
    while (it != map_orphan_txs_.end())
    {
        auto maybe_erase = it++; // increment to avoid iterator becoming invalid
        if (maybe_erase->second.from_peer == id)
        {
            erased_count += EraseOrphanTx(maybe_erase->second.tx->Hash());
        }
    }
    if (erased_count > 0)
        BTCLOG(LOG_LEVEL_INFO) << "Erased " << erased_count << "orphan tx from peer " << id;
}

uint32_t Orphans::LimitOrphanTxSize(uint32_t max_orphans)
{
    LOCK(cs_orphans_);

    uint32_t evicted_count = 0;
    static int64_t next_sweep;
    int64_t now = Time::GetTimeSeconds();
    if (next_sweep <= now) {
        // Sweep out expired orphan pool entries:
        int erased_count = 0;
        int64_t min_expire_time = now + kBrphanTxExpireTime - kOrphanTxExpireInterval;
        auto it = map_orphan_txs_.begin();
        while (it != map_orphan_txs_.end())
        {
            auto maybe_erase = it++;
            if (maybe_erase->second.time_expire <= now) {
                erased_count += EraseOrphanTx(maybe_erase->second.tx->Hash());
            } else {
                min_expire_time = std::min(maybe_erase->second.time_expire, min_expire_time);
            }
        }
        // Sweep again 5 minutes after the next entry that expires in order to batch the linear scan.
        next_sweep = min_expire_time + kOrphanTxExpireInterval;
        if (erased_count > 0)
            BTCLOG(LOG_LEVEL_INFO) << "Erased " << erased_count << "orphan tx due to expiration";
    }
    
    while (map_orphan_txs_.size() > max_orphans)
    {
        // Evict a random orphan:
        Uint256 randomhash = Random::GetUint256();
        auto it = map_orphan_txs_.lower_bound(randomhash);
        if (it == map_orphan_txs_.end())
            it = map_orphan_txs_.begin();
        EraseOrphanTx(it->first);
        ++evicted_count;
    }
    
    return evicted_count;
}

int Orphans::EraseOrphanTx(const Hash256& hash)
{
    auto it = map_orphan_txs_.find(hash);
    if (it == map_orphan_txs_.end())
        return 0;
    
    for (const TxIn& txin : it->second.tx->inputs())
    {
        auto it_prev = map_prev_orphan_txs_.find(txin.prevout());
        if (it_prev == map_prev_orphan_txs_.end())
            continue;
        it_prev->second.erase(it);
        if (it_prev->second.empty())
            map_prev_orphan_txs_.erase(it_prev);
    }
    map_orphan_txs_.erase(it);
    
    return 1;
}

