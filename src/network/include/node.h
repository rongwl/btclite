#ifndef BTCLITE_NODE_H
#define BTCLITE_NODE_H


#include <event2/bufferevent.h>
#include <functional>
#include <queue>

#include "block_sync.h"
#include "bloom.h"
#include "timer.h"


// Ping time measurement
struct PingTime {
    // The pong reply we're expecting, or 0 if no pong expected.
    std::atomic<uint64_t> ping_nonce_sent;
    
    // Time (in usec) the last ping was sent, or 0 if no ping was ever sent.
    std::atomic<int64_t> ping_usec_start;
    
    // Last measured round-trip time.
    std::atomic<int64_t> ping_usec_time;
    
    // Best measured round-trip time.
    std::atomic<int64_t> min_ping_usec_time;
    
    // Whether a ping is requested.
    std::atomic<bool> ping_queued;
};

struct NodeTimers {
    TimerMng::TimerPtr no_msg_timer;
    TimerMng::TimerPtr no_sending_timer;
    TimerMng::TimerPtr no_receiving_timer;
    TimerMng::TimerPtr no_ping_timer;
    TimerMng::TimerPtr no_connection_timer;
};

/* Information about a connected peer */
class Node {
public:
    Node(const struct bufferevent *bev, const btclite::NetAddr& addr,
         bool is_inbound = true, bool manual = false, std::string host_name = "");
    ~Node();
    
    //-------------------------------------------------------------------------
    static void InactivityTimeoutCb(std::shared_ptr<Node> node);
    bool ParseMessage();
    bool SendMessage(const BaseMsgType& msg);
    
    //-------------------------------------------------------------------------
    int64_t time_connected() const
    {
        return time_connected_;
    }
    
    NodeId id() const
    {
        return id_;
    }
    
    int version() const
    {
        return version_;
    }
    
    ServiceFlags services() const
    {
        return services_;
    }
    
    const struct bufferevent* const bev() const
    {
        return bev_;
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
    
    bool manual() const
    {
        return manual_;
    }
    
    bool conn_established() const
    {
        return conn_established_;
    }
    
    bool disconnected() const
    {
        return disconnected_;
    }
    
    void set_disconnected(bool disconnected)
    {
        disconnected_ = disconnected;
    }
    
    int64_t time_last_send() const
    {
        return time_last_send_;
    }
    
    int64_t time_last_recv() const
    {
        return time_last_recv_;
    }
    
    std::string host_name() const
    {
        LOCK(cs_host_name_);
        return host_name_;
    }
    
    void set_host_name(const std::string& name)
    {
        LOCK(cs_host_name_);
        host_name_ = name;
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
    
    const PingTime& ping_time() const
    {
        return ping_time_;
    }
    
    int64_t last_block_time() const
    {
        return last_block_time_;
    }
    
    int64_t last_tx_time() const
    {
        return last_tx_time_;
    }
    
    const NodeTimers& timers() const
    {
        return timers_;
    }
    
    NodeTimers *mutable_timers()
    {
        return &timers_;
    }
    
private:
    const int64_t time_connected_;
    const NodeId id_;
    std::atomic<int> version_;
    const ServiceFlags services_;
    const int start_height_;
    struct bufferevent *bev_;
    const btclite::NetAddr addr_;
    //const uint64_t keyed_net_group_;
    const uint64_t local_host_nonce_;
    
    std::atomic<int64_t> time_last_send_;
    std::atomic<int64_t> time_last_recv_;
    
    mutable CriticalSection cs_host_name_;
    std::string host_name_;
    
    const bool is_inbound_;
    const bool manual_;
    std::atomic<bool> conn_established_;
    std::atomic<bool> disconnected_;
    //SemaphoreGrant grant_outbound_;
    
    mutable CriticalSection cs_bloom_filter_;
    std::unique_ptr<BloomFilter> bloom_filter_;
    // We use fRelayTxes for two purposes -
    // a) it allows us to not relay tx invs before receiving the peer's version message
    // b) the peer may tell us in its version message that we should not relay tx invs
    //    unless it loads a bloom filter.
    bool relay_txes_; // protected by cs_bloom_filter_
    
    // Ping time measurement
    PingTime ping_time_;
    
    // Block and TXN accept times
    std::atomic<int64_t> last_block_time_;
    std::atomic<int64_t> last_tx_time_;
    
    NodeTimers timers_;
};

struct NodeEvictionCandidate
{
    NodeId id;
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
    
    NodeId GetNewNodeId()
    {
        static std::atomic<NodeId> last_node_id = 0;
        return last_node_id.fetch_add(1, std::memory_order_relaxed);
    }
    
    size_t Size() const
    {
        LOCK(cs_nodes_);
        return list_.size();
    }
    
    //-------------------------------------------------------------------------
    void AddNode(std::shared_ptr<Node> node);    
    std::shared_ptr<Node> InitializeNode(const struct bufferevent *bev, const btclite::NetAddr& addr,
                                         bool is_inbound = true, bool manual = false);
    std::shared_ptr<Node> GetNode(NodeId id);    
    std::shared_ptr<Node> GetNode(struct bufferevent *bev);    
    std::shared_ptr<Node> GetNode(const btclite::NetAddr& addr);    
    void EraseNode(std::shared_ptr<Node> node);    
    void EraseNode(NodeId id);
    
    //-------------------------------------------------------------------------
    void Clear()
    {
        LOCK(cs_nodes_);
        list_.clear();
    }
    
    void ClearDisconnected();    
    void DisconnectNode(const SubNet& subnet);
    //bool AttemptToEvictConnection();
    int CountInbound();
    int CountOutbound();
    bool ShouldDnsLookup();
    
private:
    mutable CriticalSection cs_nodes_;
    std::list<std::shared_ptr<Node> > list_;

    //void MakeEvictionCandidate(std::vector<NodeEvictionCandidate> *out);
};

// Singleton pattern, thread safe after c++11
class SingletonNodes : Uncopyable {
public:
    static Nodes& GetInstance()
    {
        static Nodes nodes;
        return nodes;
    }
    
private:
    SingletonNodes() {}    
};

class MsgHandler {
public:
    using MsgDataHandler = std::function<bool()>;
    
    MsgHandler(std::shared_ptr<Message> msg, std::shared_ptr<Node> src_node)
    {
        Factory(msg, src_node);
    }
    
    static bool HandleMessage(std::shared_ptr<Message> msg, std::shared_ptr<Node> src_node,
                              MsgDataHandler data_handler);
    
    std::function<bool()> data_handler() const
    {
        return data_handler_;
    }
    
private:
    MsgDataHandler data_handler_;
    
    void Factory(std::shared_ptr<Message> msg, std::shared_ptr<Node> src_node);

    static bool VerifyMsgHeader(const MessageHeader& header);
    static bool HandleRecvVersion(std::shared_ptr<Message> msg, std::shared_ptr<Node> src_node);
};


#endif // BTCLITE_NODE_H
