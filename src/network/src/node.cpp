#include "node.h"

#include "bandb.h"
#include "chain.h"
#include "net.h"
#include "peers.h"
#include "thread.h"


namespace btclite {
namespace network {

const BloomFilter *NodeFilter::bloom_filter() const
{
    LOCK(cs_bloom_filter_);
    
    if (!bloom_filter_)
        return nullptr;
    
    return bloom_filter_.get();
}

Node::Node(const struct bufferevent *bev, const btclite::network::NetAddr& addr,
           bool is_inbound, bool manual, std::string host_name)
    : id_(SingletonNodes::GetInstance().GetNewNodeId()),
      services_(SingletonLocalNetCfg::GetInstance().local_services()),
      bev_(const_cast<struct bufferevent*>(bev)), addr_(addr),
      local_host_nonce_(btclite::utility::GetUint64()),
      host_name_(host_name), is_inbound_(is_inbound), manual_(manual), known_addrs_(3000),
      time_(btclite::utility::util_time::GetTimeSeconds())
{
    time_.ping_time.min_ping_usec_time = std::numeric_limits<int64_t>::max();
}

Node::~Node()
{
    if (bev_)
        bufferevent_free(bev_);
    if (!SingletonNetInterrupt::GetInstance()) {
        if (SingletonBlockSync::GetInstance().ShouldUpdateTime(id_))
            SingletonPeers::GetInstance().UpdateTime(addr_);
        
        if (SingletonBlockSync::GetInstance().IsExist(id_)) {
            auto task = std::bind(&BlockSync::EraseSyncState, &(SingletonBlockSync::GetInstance()), std::placeholders::_1);
            SingletonThreadPool::GetInstance().AddTask(std::function<void(NodeId)>(task), id_);
        }
    }
}

void Node::InactivityTimeoutCb(std::shared_ptr<Node> node)
{
    if (SingletonNetInterrupt::GetInstance())
        return;

    BTCLOG(LOG_LEVEL_WARNING) << "Peer " << node->id() << " inactive timeout.";
    node->set_disconnected(true);
}

bool Node::CheckBanned()
{
    BlockSync &block_sync = SingletonBlockSync::GetInstance();
    bool should_ban = false;
    
    if (!block_sync.GetShouldBan(id_, &should_ban) || !should_ban)
        return false;
    
    block_sync.SetShouldBan(id_, false);
    if (manual_) {
        BTCLOG(LOG_LEVEL_WARNING) << "Can not punishing manually-connected peer "
                                  << addr_.ToString();
    }
    else {
        set_disconnected(true);
        if (addr_.IsLocal()) {
            BTCLOG(LOG_LEVEL_WARNING) << "Can not banning local peer "
                                      << addr_.ToString();
        }
        else {
            SingletonBanDb::GetInstance().Add(addr_, BanDb::BanReason::NodeMisbehaving);
        }
    }
    
    return true;
}

bool Node::PushAddrToSend(const btclite::network::NetAddr& addr)
{
    LOCK(cs_addrs_);
    
    if (!addr.IsValid() || 
            std::find(known_addrs_.begin(), known_addrs_.end(), 
                      addr.GetHash().GetLow64()) != known_addrs_.end())
        return false;
    
    if (addrs_to_send_.size() >= kMaxAddrToSend) {
        addrs_to_send_[btclite::utility::GetUint64(addrs_to_send_.size())] = addr;
    } else {
        addrs_to_send_.push_back(addr);
    }
    
    return true;
}

void Node::ClearSentAddr()
{
    LOCK(cs_addrs_);
    addrs_to_send_.clear();
    
    // we only send the big addr message once
    if (addrs_to_send_.capacity() > 40)
        addrs_to_send_.shrink_to_fit();
}

bool Node::AddKnownAddr(const NetAddr& addr)
{
    LOCK(cs_addrs_);
    
    if (!addr.IsValid())
        return false;
    
    known_addrs_.push_back(addr.GetHash().GetLow64());
    
    return true;
}

void Node::set_disconnected(bool disconnected)
{
    disconnected_ = disconnected;
    SingletonNodes::GetInstance().EraseNode(id_);
}

void Nodes::AddNode(std::shared_ptr<Node> node)
{
    LOCK(cs_nodes_);
    list_.push_back(node);
    BTCLOG(LOG_LEVEL_VERBOSE) << "Added node, id:" << node->id() << " addr:" << node->addr().ToString();
}

std::shared_ptr<Node> Nodes::InitializeNode(const struct bufferevent *bev, const btclite::network::NetAddr& addr,
                                            bool is_inbound, bool manual)
{
    auto node = std::make_shared<Node>(bev, addr, is_inbound, manual);
    node->mutable_timers()->no_msg_timer = SingletonTimerMng::GetInstance().
                                           StartTimer(kNoMsgTimeout*1000, 0, Node::InactivityTimeoutCb, node);
    
    AddNode(node);    
    if (!GetNode(node->id())) {
        BTCLOG(LOG_LEVEL_WARNING) << "Save new node to nodes failed.";
        return nullptr;
    }
    
    SingletonBlockSync::GetInstance().AddSyncState(node->id(), node->addr(), node->host_name());
    
    return node;
}

std::shared_ptr<Node> Nodes::GetNode(NodeId id)
{
    LOCK(cs_nodes_);
    for (auto it = list_.begin(); it != list_.end(); ++it)
        if ((*it)->id() == id)
            return (*it);
    
    return nullptr;
}

std::shared_ptr<Node> Nodes::GetNode(struct bufferevent *bev)
{
    LOCK(cs_nodes_);
    for (auto it = list_.begin(); it != list_.end(); ++it)
        if ((*it)->bev() == bev)
            return (*it);
    
    return nullptr;
}

std::shared_ptr<Node> Nodes::GetNode(const btclite::network::NetAddr& addr)
{
    LOCK(cs_nodes_);
    for (auto it = list_.begin(); it != list_.end(); ++it)
        if ((*it)->addr() == addr)
            return (*it);
    
    return nullptr;
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

void Nodes::ClearDisconnected()
{
    LOCK(cs_nodes_);
    
    for (auto it = list_.begin(); it != list_.end(); ++it) {
        if ((*it)->disconnected()) {
        
        }
    }
}

void Nodes::DisconnectNode(const SubNet& subnet)
{
    LOCK(cs_nodes_);
    
    for (auto it = list_.begin(); it != list_.end(); ++it) {
        if (subnet.Match((*it)->addr()))
            (*it)->set_disconnected(true);
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
int Nodes::CountInbound()
{
    int num = 0;
    
    LOCK(cs_nodes_);    
    for (auto it = list_.begin(); it != list_.end(); ++it) {
        if ((*it)->is_inbound())
            num++;
    }
    
    return num;
}

int Nodes::CountOutbound()
{
    LOCK(cs_nodes_);
    return list_.size() - CountInbound();
}

bool Nodes::ShouldDnsLookup()
{
    int count = 0;
    
    LOCK(cs_nodes_);
    for (auto it = list_.begin(); it != list_.end(); ++it) {
        if ((*it)->conn_established() && !(*it)->manual() && !(*it)->is_inbound())
            count++;
    }
    
    return (count < 2);
}

bool Nodes::CheckIncomingNonce(uint64_t nonce)
{
    LOCK(cs_nodes_);
    for (auto it = list_.begin(); it != list_.end(); ++it) {
        if (!(*it)->conn_established() && !(*it)->is_inbound() &&
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

} // namespace network
} // namespace btclite
