#include "node.h"

#include <event2/buffer.h>

#include "chain.h"
#include "peers.h"
#include "thread.h"


Node::Node(const struct bufferevent *bev, const btclite::network::NetAddr& addr,
           bool is_inbound, bool manual, std::string host_name)
    : time_connected_(Time::GetTimeSeconds()),
      id_(SingletonNodes::GetInstance().GetNewNodeId()),
      version_(0),
      services_(SingletonLocalNetCfg::GetInstance().local_services()),
      start_height_(SingletonBlockChain::GetInstance().Height()),
      bev_(const_cast<struct bufferevent*>(bev)),
      addr_(addr),
      local_host_nonce_(Random::GetUint64(std::numeric_limits<uint64_t>::max())),
      time_last_send_(0),
      time_last_recv_(0),
      is_inbound_(is_inbound),
      manual_(manual),
      host_name_(host_name),
      conn_established_(false),
      disconnected_(false),
      bloom_filter_(std::make_unique<BloomFilter>()),
      ping_time_({ 0, 0, 0, std::numeric_limits<int64_t>::max(), false }),
      last_block_time_(0),
      last_tx_time_(0),
      timers_()
{

}

Node::~Node()
{
    if (bev_)
        bufferevent_free(bev_);
    if (!SingletonNetInterrupt::GetInstance()) {
        if (SingletonBlockSync::GetInstance().ShouldUpdateTime(id_))
            SingletonPeers::GetInstance().UpdateTime(addr_);
        
        auto task = std::bind(&BlockSync::EraseSyncState, &(SingletonBlockSync::GetInstance()), std::placeholders::_1);
        SingletonThreadPool::GetInstance().AddTask(std::function<void(NodeId)>(task), id_);
    }
}

void Node::InactivityTimeoutCb(std::shared_ptr<Node> node)
{
    if (SingletonNetInterrupt::GetInstance())
        return;

    BTCLOG(LOG_LEVEL_WARNING) << "Node " << node->id() << " inactive timeout.";
    node->set_disconnected(true);
    SingletonNodes::GetInstance().EraseNode(node);
}

bool Node::ParseMessage()
{
    struct evbuffer *buf;
    uint8_t *raw;
    
    if (!bev_)
        return false;
    
    if (nullptr == (buf = bufferevent_get_input(bev_)))
        return false;
    
    if (disconnected_)
        return false;
    
    raw = evbuffer_pullup(buf, MessageHeader::kSize);
    while (raw) {
        MessageHeader header(raw);
        
        if (!header.IsValid()) {
            BTCLOG(LOG_LEVEL_ERROR) << "Received invalid message header from peer " << id_;
            return false;
        }
        
        raw = evbuffer_pullup(buf, header.payload_length());
        auto pmessage = std::make_shared<Message>(std::move(header), raw);
        
        std::shared_ptr<Node> pnode = SingletonNodes::GetInstance().GetNode(id_);
        assert(pnode != nullptr);
        
        MsgHandler handler(pmessage, pnode);
        auto task = std::bind(MsgHandler::HandleMessage, pmessage, pnode, handler.data_handler());
        SingletonThreadPool::GetInstance().AddTask(std::function<bool()>(task));
        
        raw = evbuffer_pullup(buf, MessageHeader::kSize);
    }
    
    return true;
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
    for (auto it = list_.begin(); it != list_.end(); ++it)
        if ((*it)->id() == id) {
            list_.erase(it);
            BTCLOG(LOG_LEVEL_VERBOSE) << "Cleared node " << id;
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

void MsgHandler::Factory(std::shared_ptr<Message> msg, std::shared_ptr<Node> src_node)
{
    if (msg->header().command() == kMsgVersion) {
        data_handler_ = std::move(std::function<bool()>(std::bind(HandleRecvVersion, msg, src_node)));
    }
}

bool MsgHandler::HandleMessage(std::shared_ptr<Message> msg, std::shared_ptr<Node> src_node,
                              MsgDataHandler data_handler)
{
    bool ret = false;
    
    if (src_node->disconnected())
        return false;
        
    if (!VerifyMsgHeader(msg->header()))
        return false;
    
    try {
        ret = data_handler();
    }
    catch (const std::exception& e) {
        
    }
    catch (...) {
        
    }
    
    return ret;
}

bool MsgHandler::VerifyMsgHeader(const MessageHeader& header)
{
    return true;
}

bool MsgHandler::HandleRecvVersion(std::shared_ptr<Message> msg, std::shared_ptr<Node> src_node)
{
    bool ret = false;
    
    
    
    return ret;
}
