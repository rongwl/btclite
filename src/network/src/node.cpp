#include "node.h"

#include "banlist.h"
#include "msg_process.h"
#include "peers.h"
#include "protocol/ping.h"
#include "thread.h"


namespace btclite {
namespace network {

bool IsInitialBlockDownload()
{
    return false;
}

bool FloodingAddrs::PushAddrToSend(const NetAddr& addr)
{
    LOCK(cs_);
    
    if (!addr.IsValid() || 
            std::find(known_addrs_.begin(), known_addrs_.end(), 
                      addr.GetHash().GetLow64()) != known_addrs_.end())
        return false;
    
    if (addrs_to_send_.size() >= kMaxAddrToSend) {
        addrs_to_send_[util::RandUint64(addrs_to_send_.size())] = addr;
    } else {
        addrs_to_send_.push_back(addr);
    }
    
    return true;
}

void FloodingAddrs::ClearSentAddr()
{
    LOCK(cs_);
    addrs_to_send_.clear();
    
    // we only send the big addr message once
    if (addrs_to_send_.capacity() > 40)
        addrs_to_send_.shrink_to_fit();
}

bool FloodingAddrs::AddKnownAddr(const NetAddr& addr)
{
    LOCK(cs_);
    
    if (!addr.IsValid())
        return false;
    
    known_addrs_.push_back(addr.GetHash().GetLow64());
    
    return true;
}

const BloomFilter *NodeFilter::bloom_filter() const
{
    LOCK(cs_);
    
    if (!bloom_filter_)
        return nullptr;
    
    return bloom_filter_.get();
}

void Misbehavior::Misbehaving(NodeId id, int howmuch)
{
    if (howmuch == 0)
        return;
    
    LOCK(cs_);
    
    score_ += howmuch;
    if (score_ >= kDefaultBanscoreThreshold &&
            score_ - howmuch < kDefaultBanscoreThreshold) {
        should_ban_ = true;
        BTCLOG(LOG_LEVEL_INFO) << "ban threshold exceeded, peer:" << id 
                               << " misbehavior_score:" << score_;
    }
    else
        BTCLOG(LOG_LEVEL_INFO) << "peer " << id << " misbehavior, score:" << score_;
}

Node::Node(const struct bufferevent *bev, const NetAddr& addr,
           bool is_inbound, bool manual, std::string host_name)
    : id_(GetNewNodeId()), is_inbound_(is_inbound), 
      local_host_nonce_(util::RandUint64()),
      connection_(bev, addr, manual, host_name),
      time_(util::GetTimeSeconds())
{
    int& num_prefered_download = NumPreferedDownload();
    num_prefered_download += IsPreferedDownload();
    time_.ping_time.min_ping_usec_time = std::numeric_limits<int64_t>::max();
}

void Node::StopAllTimers()
{
    util::TimerMng& timer_mng = util::SingletonTimerMng::GetInstance();    
    
    if (timers_.no_msg_timer) {
        timer_mng.StopTimer(timers_.no_msg_timer);
        timers_.no_msg_timer.reset();
    }
    
    if (timers_.no_sending_timer) {
        timer_mng.StopTimer(timers_.no_sending_timer);
        timers_.no_sending_timer.reset();
    }
    
    if (timers_.no_receiving_timer) {
        timer_mng.StopTimer(timers_.no_receiving_timer);
        timers_.no_receiving_timer.reset();
    }
    
    if (timers_.shakehands_timer) {
        timer_mng.StopTimer(timers_.shakehands_timer);
        timers_.shakehands_timer.reset();
    }
    
    if (timers_.ping_timer) {
        timer_mng.StopTimer(timers_.ping_timer);
        timers_.ping_timer.reset();
    }
    
    if (timers_.advertise_local_addr_timer) {
        timer_mng.StopTimer(timers_.advertise_local_addr_timer);
        timers_.advertise_local_addr_timer.reset();
    }
    
    if (timers_.broadcast_addrs_timer) {
        timer_mng.StopTimer(timers_.broadcast_addrs_timer);
        timers_.broadcast_addrs_timer.reset();
    }
}

bool Node::CheckBanned(BanList *pbanlist)
{
    if (!misbehavior_.should_ban())
        return false;
    
    misbehavior_.set_should_ban(false);
    if (connection_.manual()) {
        BTCLOG(LOG_LEVEL_WARNING) << "Can not punishing manually-connected peer "
                                  << connection_.addr().ToString();
    }
    else {
        connection_.Disconnect();
        if (connection_.addr().IsLocal()) {
            BTCLOG(LOG_LEVEL_WARNING) << "Can not banning local peer "
                                      << connection_.addr().ToString();
        }
        else {
            if (pbanlist) {
                pbanlist->Add(connection_.addr(), BanList::BanReason::kNodeMisbehaving);
            }
        }
    }
    
    return true;
}

void Node::InactivityTimeoutCb()
{
    if (util::SingletonInterruptor::GetInstance() || 
            connection_.IsDisconnected()) {
        return;
    }

    BTCLOG(LOG_LEVEL_WARNING) << "Peer " << id_ << " inactive timeout.";
    connection_.Disconnect();
}

void Node::SocketNoMsgTimeoutCb()
{
    if (util::SingletonInterruptor::GetInstance() || 
            connection_.IsDisconnected()) {
        return;
    }
    
    if (!connection_.socket_no_msg())
        return;
    
    BTCLOG(LOG_LEVEL_WARNING) << "Socket no message in first " << kNoMsgTimeout 
                              << " seconds from peer " << id_;
    connection_.Disconnect();
}

void Node::PingTimeoutCb(uint32_t magic)
{
    if (util::SingletonInterruptor::GetInstance() || 
            connection_.IsDisconnected()) {
        return;
    }
    
    if (time_.ping_time.ping_nonce_sent) {
        BTCLOG(LOG_LEVEL_WARNING) << "Peer " << id_ << " ping timeout.";
        util::SingletonTimerMng::GetInstance().StopTimer(timers_.ping_timer);
        connection_.Disconnect();
        return;
    }
    
    // send ping
    protocol::Ping ping(util::RandUint64());
    if (protocol_.version < kBip31Version)
        ping.set_protocol_version(0);
    SendMsg(ping, magic, this->shared_from_this());
}

void Node::ShakeHandsTimeoutCb()
{
    if (util::SingletonInterruptor::GetInstance() || 
            connection_.IsDisconnected()) {
        return;
    }
    
    if (connection_.IsHandshakeCompleted()) {
        return;
    }
    
    BTCLOG(LOG_LEVEL_WARNING) << "Peer "<< id_ << " shakehands timeout.";
    connection_.Disconnect();
}


void Nodes::AddNode(std::shared_ptr<Node> node)
{
    LOCK(cs_nodes_);
    list_.push_back(node);
    BTCLOG(LOG_LEVEL_VERBOSE) << "Added node, id:" << node->id() 
                              << " addr:" << node->connection().addr().ToString();
}

std::shared_ptr<Node> Nodes::InitializeNode(const struct bufferevent *bev, 
        const NetAddr& addr,
        bool is_inbound, bool manual)
{
    auto node = std::make_shared<Node>(bev, addr, is_inbound, manual);
    
    util::TimerMng& timer_mng = util::SingletonTimerMng::GetInstance();
    node->mutable_timers()->no_msg_timer = timer_mng.StartTimer(
            kNoMsgTimeout*1000, 0, std::bind(&Node::SocketNoMsgTimeoutCb, node));
    node->mutable_timers()->shakehands_timer = timer_mng.StartTimer(
                kShakeHandsTimeout*1000, 0, std::bind(&Node::ShakeHandsTimeoutCb, node));
    
    AddNode(node);
    if (!GetNode(node->id())) {
        BTCLOG(LOG_LEVEL_WARNING) << "Save new node to nodes failed.";
        return nullptr;
    }   
    
    return node;
}

std::shared_ptr<Node> Nodes::GetNode(NodeId id) const
{
    LOCK(cs_nodes_);
    for (auto it = list_.begin(); it != list_.end(); ++it)
        if ((*it)->id() == id)
            return (*it);
    
    return nullptr;
}

std::shared_ptr<Node> Nodes::GetNode(struct bufferevent *bev) const
{
    LOCK(cs_nodes_);
    for (auto it = list_.begin(); it != list_.end(); ++it)
        if ((*it)->connection().bev() == bev)
            return (*it);
    
    return nullptr;
}

std::shared_ptr<Node> Nodes::GetNode(const NetAddr& addr) const
{
    LOCK(cs_nodes_);
    for (auto it = list_.begin(); it != list_.end(); ++it)
        if ((*it)->connection().addr() == addr)
            return (*it);
    
    return nullptr;
}

void Nodes::GetNode(const SubNet& subnet, std::vector<std::shared_ptr<Node> > *out) const
{
    if (!out)
        return;
    
    out->clear();    
    LOCK(cs_nodes_);
    for (auto it = list_.begin(); it != list_.end(); ++it) {
        if (subnet.Match((*it)->connection().addr())) {
            out->push_back(*it);
        }
    }
}

void Nodes::EraseNode(std::shared_ptr<Node> node)
{
    LOCK(cs_nodes_);
    auto it = std::find(list_.begin(), list_.end(), node);
    if (it != list_.end()) {
        list_.erase(it);
        BTCLOG(LOG_LEVEL_VERBOSE) << "Cleared node " << node->id();
    }
}

void Nodes::EraseNode(NodeId id)
{
    LOCK(cs_nodes_);
    for (auto it = list_.begin(); it != list_.end(); ++it) {
        if ((*it)->id() == id) {
            list_.erase(it);
            BTCLOG(LOG_LEVEL_VERBOSE) << "Cleared node " << id;
            break;
        }
    }
}


/*
static bool ReverseCompareNodeMinPingTime(const NodeEvictionCandidate &a, const NodeEvictionCandidate &b)
{
    return a.min_ping_usec_time > b.min_ping_usec_time;
}

static bool ReverseCompareNodeTimeConnected(const NodeEvictionCandidate &a, const NodeEvictionCandidate &b)
{
    return a.time_connected > b.time_connected;
}

static bool CompareNetGroupKeyed(const NodeEvictionCandidate &a, const NodeEvictionCandidate &b) {
    return a.keyed_net_group < b.keyed_net_group;
}

static bool CompareNodeBlockTime(const NodeEvictionCandidate &a, const NodeEvictionCandidate &b)
{
    // There is a fall-through here because it is common for a node to have many peers which have not yet relayed a block.
    if (a.last_block_time != b.last_block_time) 
        return a.last_block_time < b.last_block_time;
    if (a.relevant_services != b.relevant_services)
        return b.relevant_services;
     
    return a.time_connected > b.time_connected;
}

static bool CompareNodeTXTime(const NodeEvictionCandidate &a, const NodeEvictionCandidate &b)
{
    // There is a fall-through here because it is common for a node to have more than a few peers that have not yet relayed txn.
    if (a.last_tx_time != b.last_tx_time)
        return a.last_tx_time < b.last_tx_time;
    if (a.relay_txes != b.relay_txes)
        return b.relay_txes;
    if (a.bloom_filter != b.bloom_filter)
        return a.bloom_filter;
     
    return a.time_connected > b.time_connected;
}
*/
//! Sort an array by the specified comparator, then erase the last K elements.
template<typename T, typename Comparator>
static void EraseLastKElements(std::vector<T> &elements, Comparator comparator, size_t k)
{
    std::sort(elements.begin(), elements.end(), comparator);
    size_t erase_size = std::min(k, elements.size());
    elements.erase(elements.end() - erase_size, elements.end());
}

/*  Merge from bitcoin core 0.16.3
 *  Try to find a connection to evict when the node is full.
 *  Extreme care must be taken to avoid opening the node to attacker
 *   triggered network partitioning.
 *  The strategy used here is to protect a small number of peers
 *   for each of several distinct characteristics which are difficult
 *   to forge.  In order to partition a node the attacker must be
 *   simultaneously better at all of them than honest peers.
 */
/*
bool Nodes::AttemptToEvictConnection()
{
    std::vector<NodeEvictionCandidate> eviction_candidates;
    MakeEvictionCandidate(&eviction_candidates);
     
    // Protect connections with certain characteristics

    // Deterministically select 4 peers to protect by netgroup.
    // An attacker cannot predict which netgroups will be protected
    EraseLastKElements(eviction_candidates, CompareNetGroupKeyed, 4);
    // Protect the 8 nodes with the lowest minimum ping time.
    // An attacker cannot manipulate this metric without physically moving nodes closer to the target.
    EraseLastKElements(eviction_candidates, ReverseCompareNodeMinPingTime, 8);
    // Protect 4 nodes that most recently sent us transactions.
    // An attacker cannot manipulate this metric without performing useful work.
    EraseLastKElements(eviction_candidates, CompareNodeTXTime, 4);
    // Protect 4 nodes that most recently sent us blocks.
    // An attacker cannot manipulate this metric without performing useful work.
    EraseLastKElements(eviction_candidates, CompareNodeBlockTime, 4);
    // Protect the half of the remaining nodes which have been connected the longest.
    // This replicates the non-eviction implicit behavior, and precludes attacks that start later.
    EraseLastKElements(eviction_candidates, ReverseCompareNodeTimeConnected, eviction_candidates.size() / 2);
     
    if (eviction_candidates.empty())
        return false;

    // Identify the network group with the most connections and youngest member.
    // (vEvictionCandidates is already sorted by reverse connect time)
    uint64_t most_connections_group;
    unsigned int most_connections = 0;
    int64_t most_connections_time = 0;
    std::map<uint64_t, std::vector<NodeEvictionCandidate> > map_netgroup_nodes;
    for (const NodeEvictionCandidate &node : eviction_candidates) {
        std::vector<NodeEvictionCandidate> &group = map_netgroup_nodes[node.keyed_net_group];
        group.push_back(node);
        int64_t grouptime = group[0].time_connected;

        if (group.size() > most_connections || (group.size() == most_connections && grouptime > most_connections_time)) {
            most_connections = group.size();
            most_connections_time = grouptime;
            most_connections_group = node.keyed_net_group;
        }
    }
     
    // Reduce to the network group with the most connections
    eviction_candidates = std::move(map_netgroup_nodes[most_connections_group]);
    if (!DisconnectNode(eviction_candidates.front().id))
        return false;
     
    return true;
}
*/


int Nodes::CountPreferredDownload() const
{
    int num = 0;
    
    LOCK(cs_nodes_);
    for (auto it = list_.begin(); it != list_.end(); ++it) {
        if (!(*it)->connection().IsDisconnected() &&
                (*it)->IsPreferedDownload())
            num++;
    }
    
    return num;
}

bool Nodes::ShouldDnsLookup() const
{
    int count = 0;
    
    LOCK(cs_nodes_);
    for (auto it = list_.begin(); it != list_.end(); ++it) {
        if ((*it)->connection().IsHandshakeCompleted() && 
                !(*it)->connection().manual() && 
                !(*it)->is_inbound())
            count++;
    }
    
    return (count < 2);
}

bool Nodes::CheckIncomingNonce(uint64_t nonce) const
{
    LOCK(cs_nodes_);
    for (auto it = list_.begin(); it != list_.end(); ++it) {
        if (!(*it)->connection().IsHandshakeCompleted() && 
                !(*it)->is_inbound() &&
                (*it)->local_host_nonce() == nonce)
            return false;
    }

    return true;
}

/*
void Nodes::MakeEvictionCandidate(std::vector<NodeEvictionCandidate> *out)
{
    LOCK(cs_nodes_);
     
    for (auto it = list_.begin(); it != list_.end(); ++it) {
        if (!(*it)->is_inbound())
            continue;
        if ((*it)->disconnected())
            continue;
        NodeEvictionCandidate candidate = {(*it)->id(), (*it)->time_connected(), (*it)->min_ping_usec_time(),
                                           (*it)->last_block_time(), (*it)->last_tx_time(),
                                           HasAllDesirableServiceFlags((*it)->services()),
                                           (*it)->relay_txes(), (*it)->bloom_filter() != nullptr,
                                           (*it)->addr(), (*it)->keyed_net_group()};
        out->push_back(candidate);
    }
}
*/

void DisconnectNode(std::shared_ptr<Node> node)
{
    node->mutable_connection()->set_connection_state(NodeConnection::kDisconnected);
    node->StopAllTimers();
    
    if (node->ShouldUpdateTime())
        SingletonPeers::GetInstance().UpdateTime(node->connection().addr());
    
    for (auto& entry : node->blocks_in_flight().list()) {
        SingletonBlocksInFlight::GetInstance().Erase(entry.hash);
    }
    
    SingletonOrphans::GetInstance().EraseOrphansFor(node->id());
    
    SingletonNodes::GetInstance().EraseNode(node);
}

void DisconnectNode(const SubNet& subnet)
{
    std::vector<std::shared_ptr<Node> > nodes_vec;
    
    SingletonNodes::GetInstance().GetNode(subnet, &nodes_vec);
    for (auto it = nodes_vec.begin(); it != nodes_vec.end(); ++it) {
        DisconnectNode(*it);
    }
}

void DisconnectNodeCb(std::shared_ptr<Node> pnode, Nodes *pnodes,
                      Peers *ppeers, BlocksInFlight1 *pblocks_in_flight,
                      Orphans *porphans)
{
    if (!pnode || !pnodes) {
        return;
    }
    
    pnode->StopAllTimers();
    
    if (ppeers && pnode->ShouldUpdateTime()) {
        ppeers->UpdateTime(pnode->connection().addr());
    }
    
    if (pblocks_in_flight) {
        for (auto& entry : pnode->blocks_in_flight().list()) {
            pblocks_in_flight->Erase(entry.hash);
        }
    }
    
    if (porphans) {
        porphans->EraseOrphansFor(pnode->id());
    }
    
    pnodes->EraseNode(pnode);
}

} // namespace network
} // namespace btclite
