#include "block_sync.h"
#include "random.h"
#include "util_time.h"


namespace btclite {
namespace network {

using namespace consensus;

#if 0
void BlockSync::AddSyncState(NodeId id, const NetAddr& addr, const std::string& addr_name)
{
    LOCK(cs_block_sync_);
    map_sync_state_.emplace_hint(map_sync_state_.end(), std::piecewise_construct,
                                 std::forward_as_tuple(id), std::forward_as_tuple(addr, addr_name));
    BTCLOG(LOG_LEVEL_VERBOSE) << "Added node state, peer:" << id << " addr:" << addr.ToString();
}

// requires cs_block_sync_
BlockSyncState *BlockSync::GetSyncState(NodeId id)
{
    auto it = map_sync_state_.find(id);
    if (it != map_sync_state_.end())
        return &(it->second);
    return nullptr;
}

void BlockSync::EraseSyncState(NodeId id)
{
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return;
    
    if (state->basic_state().sync_started)
        sync_started_count_--;
    
    for (const QueuedBlock& entry : state->blocks_in_flight().list) {
        map_block_in_flight_.erase(entry.hash);
    }    
    SingletonOrphans::GetInstance().EraseOrphansFor(id);
    preferred_download_count_ -= state->download_state().preferred_download ? 1 : 0;
    validated_download_count_ -= (state->blocks_in_flight().valid_headers_count != 0) ? 1 : 0;
    assert(validated_download_count_ >= 0);
    protected_outbound_count_ -= state->time_state().timeout_state.protect ? 1 : 0;
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

void BlockSync::Clear()
{
    LOCK(cs_block_sync_);
    sync_started_count_ = 0;
    preferred_download_count_ = 0;
    validated_download_count_ = 0;
    protected_outbound_count_ = 0;
    map_sync_state_.clear();
    map_block_in_flight_.clear();
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

void BlockSync::Misbehaving(NodeId id, int howmuch)
{
    if (howmuch == 0)
        return;
    
    LOCK(cs_block_sync_);
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return;
    
    state->mutable_stats()->misbehavior_score += howmuch;
    if (state->stats().misbehavior_score >= kDefaultBanscoreThreshold &&
            state->stats().misbehavior_score - howmuch < kDefaultBanscoreThreshold) {
        state->mutable_basic_state()->should_ban = true;
        BTCLOG(LOG_LEVEL_INFO) << "ban threshold exceeded, peer:" << id << " misbehavior_score:"
                               << state->stats().misbehavior_score;
    }
    else
        BTCLOG(LOG_LEVEL_INFO) << "peer " << id << " misbehavior, score:" 
                               << state->stats().misbehavior_score;
}

bool BlockSync::GetNodeAddr(NodeId id, NetAddr *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->node_addr();
    
    return true;
}

bool BlockSync::GetConnected(NodeId id, bool *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->basic_state().connected;
    
    return true;
}

bool BlockSync::SetConnected(NodeId id, bool connected)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_basic_state()->connected = connected;
    
    return true;
}

bool BlockSync::GetSyncStarted(NodeId id, bool *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->basic_state().sync_started;
    
    return true;
}

bool BlockSync::SetSyncStarted(NodeId id, bool sync_started)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_basic_state()->sync_started = sync_started;
    
    return true;
}

bool BlockSync::GetShouldBan(NodeId id, bool *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->basic_state().should_ban;
    
    return true;
}

bool BlockSync::SetShouldBan(NodeId id, bool should_ban)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_basic_state()->should_ban = should_ban;
    
    return true;
}

bool BlockSync::GetMisbehaviorScore(NodeId id, int *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->stats().misbehavior_score;
    
    return true;
}

bool BlockSync::SetMisbehaviorScore(NodeId id, int misbehavior_score)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_stats()->misbehavior_score = misbehavior_score;
    
    return true;
}

bool BlockSync::GetUnconnectingHeadersLen(NodeId id, int *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->stats().unconnecting_headers_len;
    
    return true;
}

bool BlockSync::SetUnconnectingHeadersLen(NodeId id, int unconnecting_headers_len)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_stats()->unconnecting_headers_len = unconnecting_headers_len;
    
    return true;
}

bool BlockSync::GetRejects(NodeId id, std::vector<BlockReject> *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->stats().rejects;
    
    return true;
}

bool BlockSync::AddBlockReject(NodeId id, const BlockReject& reject)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_stats()->rejects.push_back(reject);
    
    return true;
}

bool BlockSync::ClearRejects(NodeId id)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_stats()->rejects.clear();
    
    return true;
}

bool BlockSync::GetTimeOut(NodeId id, int64_t *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->time_state().timeout_state.timeout;
    
    return true;
}

bool BlockSync::SetTimeout(NodeId id, int64_t timeout)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_time_state()->timeout_state.timeout = timeout;
    
    return true;
}

const BlockIndex *BlockSync::GetWorkHeader(NodeId id)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return nullptr;
    
    return state->time_state().timeout_state.work_header;
}

bool BlockSync::SetWorkHeader(NodeId id, const BlockIndex *index)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_time_state()->timeout_state.work_header = index;
    
    return true;
}

bool BlockSync::GetSentGetheaders(NodeId id, bool *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->time_state().timeout_state.sent_getheaders;
    
    return true;
}

bool BlockSync::SetSentGetheaders(NodeId id, bool sent_getheaders)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_time_state()->timeout_state.sent_getheaders = sent_getheaders;
    
    return true;
}

bool BlockSync::GetProtect(NodeId id, bool *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->time_state().timeout_state.protect;
    
    return true;
}

bool BlockSync::SetProtect(NodeId id, bool protect)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_time_state()->timeout_state.protect = protect;
    
    return true;
}

bool BlockSync::GetHeadersSyncTimeout(NodeId id, int64_t *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->time_state().headers_sync_timeout;
    
    return true;
}

bool BlockSync::SetHeadersSyncTimeout(NodeId id, int64_t headers_sync_timeout)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_time_state()->headers_sync_timeout = headers_sync_timeout;
    
    return true;
}

bool BlockSync::StallingSince(NodeId id, int64_t *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->time_state().stalling_since;
    
    return true;
}

bool BlockSync::SetStallingSince(NodeId id, int64_t stalling_since)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_time_state()->stalling_since = stalling_since;
    
    return true;
}

bool BlockSync::GetLastBlockAnnouncement(NodeId id, int64_t *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->time_state().last_block_announcement;
    
    return true;
}

bool BlockSync::SetLastBlockAnnouncement(NodeId id, int64_t last_block_announcement)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_time_state()->last_block_announcement = last_block_announcement;
    
    return true;
}

const BlockIndex *BlockSync::GetBestKnownBlockIndex(NodeId id)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return nullptr;
    
    return state->process_state().best_known_block_index;
}

bool BlockSync::SetBestKnownBlockIndex(NodeId id, const BlockIndex *index)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_process_state()->best_known_block_index = index;
    
    return true;
}

const BlockIndex *BlockSync::GetLastCommonBlockIndex(NodeId id)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return nullptr;
    
    return state->process_state().last_common_block_index;
}

bool BlockSync::SetLastCommonBlockIndex(NodeId id, const BlockIndex *index)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_process_state()->last_common_block_index = index;
    
    return true;
}

const BlockIndex *BlockSync::GetBestHeaderSentIndex(NodeId id)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return nullptr;
    
    return state->process_state().best_header_sent_index;
}

bool BlockSync::SetBestHeaderSentIndex(NodeId id, const BlockIndex *index)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_process_state()->best_header_sent_index = index;
    
    return true;
}

bool BlockSync::GetLastUnknownBlockHash(NodeId id, util::Hash256 *out) 
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->process_state().last_unknown_block_hash;
    
    return true;
}

bool BlockSync::SetLastUnknownBlockHash(NodeId id, const util::Hash256& last_unknown_block_hash)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_process_state()->last_unknown_block_hash = last_unknown_block_hash;
    
    return true;
}

bool BlockSync::ClearBlocksInFlight(NodeId id)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_blocks_in_flight()->list.clear();
    
    return true;
}

bool BlockSync::GetDownloadingSince(NodeId id, int64_t *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->blocks_in_flight().downloading_since;
    
    return true;
}

bool BlockSync::SetDownloadingSince(NodeId id, int64_t downloading_since)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_blocks_in_flight()->downloading_since = downloading_since;
    
    return true;
}

bool BlockSync::GetValidHeadersCount(NodeId id, int *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->blocks_in_flight().valid_headers_count;
    
    return true;
}

bool BlockSync::SetValidHeadersCount(NodeId id, int valid_headers_count)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_blocks_in_flight()->valid_headers_count = valid_headers_count;
    
    return true;
}

bool BlockSync::GetPreferredDownload(NodeId id, bool *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->download_state().preferred_download;
    
    return true;
}

bool BlockSync::SetPreferredDownload(NodeId id, bool preferred_download)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_download_state()->preferred_download = preferred_download;
    
    return true;
}

bool BlockSync::GetPreferHeaders(NodeId id, bool *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->download_state().prefer_headers;
    
    return true;
}

bool BlockSync::SetPreferHeaders(NodeId id, bool prefer_headers)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_download_state()->prefer_headers = prefer_headers;
    
    return true;
}

bool BlockSync::GetPreferHeaderAndIds(NodeId id, bool *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->download_state().prefer_header_and_ids;
    
    return true;
}

bool BlockSync::SetPreferHeaderAndIds(NodeId id, bool prefer_header_and_ids)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_download_state()->prefer_header_and_ids = prefer_header_and_ids;
    
    return true;
}

bool BlockSync::GetProvidesHeaderAndIds(NodeId id, bool *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->download_state().provides_header_and_ids;
    
    return true;
}

bool BlockSync::SetProvidesHeaderAndIds(NodeId id, bool provides_header_and_ids)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_download_state()->provides_header_and_ids = provides_header_and_ids;
    
    return true;
}

bool BlockSync::GetIsWitness(NodeId id, bool *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->download_state().is_witness;
    
    return true;
}

bool BlockSync::SetIsWitness(NodeId id, bool is_witness)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_download_state()->is_witness = is_witness;
    
    return true;
}

bool BlockSync::GetWantsCmpctWitness(NodeId id, bool *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->download_state().wants_cmpct_witness;
    
    return true;
}

bool BlockSync::SetWantsCmpctWitness(NodeId id, bool wants_cmpct_witness)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_download_state()->wants_cmpct_witness = wants_cmpct_witness;
    
    return true;
}

bool BlockSync::GetSupportsDesiredCmpctVersion(NodeId id, bool *out)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    *out = state->download_state().supports_desired_cmpct_version;
    
    return true;
}

bool BlockSync::SetSupportsDesiredCmpctVersion(NodeId id, bool supports_desired_cmpct_version)
{
    LOCK(cs_block_sync_);
    
    BlockSyncState *state = GetSyncState(id);
    if (!state)
        return false;
    
    state->mutable_download_state()->supports_desired_cmpct_version = supports_desired_cmpct_version;
    
    return true;
}
#endif

bool Orphans::AddOrphanTx(OrphanTx::TxSharedPtr tx, NodeId id)
{
    LOCK(cs_orphans_);

    const util::Hash256& hash = tx->GetHash();
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
            erased_count += EraseOrphanTx(maybe_erase->second.tx->GetHash());
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
    int64_t now = util::GetTimeSeconds();
    if (next_sweep <= now) {
        // Sweep out expired orphan pool entries:
        int erased_count = 0;
        int64_t min_expire_time = now + kBrphanTxExpireTime - kOrphanTxExpireInterval;
        auto it = map_orphan_txs_.begin();
        while (it != map_orphan_txs_.end())
        {
            auto maybe_erase = it++;
            if (maybe_erase->second.time_expire <= now) {
                erased_count += EraseOrphanTx(maybe_erase->second.tx->GetHash());
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
        util::Hash256 randomhash = util::RandHash256();
        auto it = map_orphan_txs_.lower_bound(randomhash);
        if (it == map_orphan_txs_.end())
            it = map_orphan_txs_.begin();
        EraseOrphanTx(it->first);
        ++evicted_count;
    }
    
    return evicted_count;
}

int Orphans::EraseOrphanTx(const util::Hash256& hash)
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


} // namespace network 
} // namespace block_sync
