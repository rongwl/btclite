#include "node.h"
#include "utiltime.h"


NodeState::NodeState(const btclite::NetAddr& addr, const std::string& addr_name)
    : address_(addr), name_(addr_name)
{
    connected_ = false;
    misbehavior_score_ = 0;
    should_ban_ = false;
    best_known_block_index_ = nullptr;
    last_unknown_block_hash_.Clear();
    last_common_block_index_ = nullptr;
    best_header_sent_index_ = nullptr;
    unconnecting_headers_len_ = 0;
    sync_started_ = false;
    headers_sync_timeout_ = 0;
    stalling_since_ = 0;
    downloading_since_ = 0;
    blocks_in_flight_ = 0;
    blocks_in_flight_valid_headers_ = 0;
    preferred_download_ = false;
    prefer_headers_ = false;
    prefer_header_and_ids_ = false;
    provides_header_and_ids_ = false;
    is_witness_ = false;
    wants_cmpct_witness_ = false;
    supports_desired_cmpct_version_ = false;
    chain_sync_ = { 0, nullptr, false, false };
    last_block_announcement_ = 0;
}

Node::Node(Id id, ServiceFlags services, int start_height, Socket::Fd sock_fd, const btclite::NetAddr& addr,
     uint64_t local_host_nonce, const btclite::NetAddr &addr_bind, const std::string& host_name, bool is_inbound)
    : time_connected_(GetTimeSeconds()), id_(id), services_(services), start_height_(start_height),
      sock_fd_(sock_fd), addr_(addr), local_host_nonce_(local_host_nonce), addr_bind_(addr_bind), 
      host_name_(host_name), is_inbound_(is_inbound), disconnected_(false), bloom_filter_(std::make_unique<BloomFilter>()),
      min_ping_usec_time_(std::numeric_limits<int64_t>::max()), last_block_time_(0), last_tx_time_(0)
{

}

void Node::Connect()
{

}

void Node::Disconnect()
{

}

size_t Node::Receive()
{
    return 0;
}

size_t Node::Send()
{
    return 0;
}

void Nodes::ClearDisconnected()
{
    LOCK(cs_nodes_);
    
    for (auto it = list_.begin(); it != list_.end(); ++it) {
        if ((*it)->disconnected()) {
        
        }
    }
}

void Nodes::ClearNodeState()
{

}

bool Nodes::DisconnectNode(Node::Id id)
{
    LOCK(cs_nodes_);
    
    for (auto it = list_.begin(); it != list_.end(); ++it) {
        if ((*it)->id() == id) {
            (*it)->set_disconnected(true);
            return true;
        }
    }
    
    return false;
}

void Nodes::DisconnectBanNode(const SubNet& subnet)
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
