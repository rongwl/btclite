#ifndef BTCLITE_NODE_H
#define BTCLITE_NODE_H


#include <event2/bufferevent.h>
#include <queue>

#include "block_sync.h"
#include "bloom.h"


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

template <typename T>
class LockQueue {
public:
    bool IsEmpty() const
    {
        LOCK(cs_queue_);
        return queue_.empty();
    }
    
    size_t Size() const
    {
        LOCK(cs_queue_);
        return queue_.size();
    }
    
    const T& Front() const
    {
        LOCK(cs_queue_);
        return queue_.front();
    }
    
    const T& Back() const
    {
        LOCK(cs_queue_);
        return queue_.front();
    }
    
    void Push(const T& val)
    {
        LOCK(cs_queue_);
        queue_.push(val);
    }
    
    void Pop()
    {
        LOCK(cs_queue_);
        queue_.pop();
    }
    
private:
    mutable CriticalSection cs_queue_;
    std::queue<T> queue_;
};

/* Information about a connected peer */
class Node {
public:    
    Node(const struct bufferevent *bev, const btclite::NetAddr& addr,
         bool is_inbound = true, std::string host_name = "");
    ~Node();
    
    //-------------------------------------------------------------------------
    static void InactivityTimeoutCb(std::shared_ptr<Node> node);
    void Connect();
    void Disconnect();
    bool ParseMessage(struct evbuffer *buf);
    size_t Send();
    
    //-------------------------------------------------------------------------
    int64_t time_connected() const
    {
        return time_connected_;
    }
    
    PeerId id() const
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
    
private:
    const int64_t time_connected_;
    const PeerId id_;
    std::atomic<int> version_;
    const ServiceFlags services_;
    const int start_height_;
    struct bufferevent *bev_socket_;
    const btclite::NetAddr addr_;
    //const uint64_t keyed_net_group_;
    const uint64_t local_host_nonce_;
    
    std::atomic<int64_t> time_last_send_;
    std::atomic<int64_t> time_last_recv_;
    
    mutable CriticalSection cs_host_name_;
    std::string host_name_;
    
    const bool is_inbound_;
    std::atomic<bool> conn_established_;
    std::atomic<bool> disconnected_;
    SemaphoreGrant grant_outbound_;
    
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
    
    LockQueue<std::shared_ptr<Message> > recv_msgs_;
};

struct NodeEvictionCandidate
{
    PeerId id;
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
    using iterator = std::list<std::shared_ptr<Node> >::iterator;
    using const_iterator = std::list<std::shared_ptr<Node> >::const_iterator;
    
    Nodes()
        : list_() {}
    
    PeerId GetNewNodeId()
    {
        static std::atomic<PeerId> last_node_id = 0;
        return last_node_id.fetch_add(1, std::memory_order_relaxed);
    }
    
    iterator Begin()
    {
        LOCK(cs_nodes_);
        return list_.begin();
    }
    const_iterator Begin() const
    {
        LOCK(cs_nodes_);
        return list_.begin();
    }
    
    iterator End()
    {
        LOCK(cs_nodes_);
        return list_.end();
    }
    const_iterator End() const
    {
        LOCK(cs_nodes_);
        return list_.end();
    }
    
    const_iterator GetNode(PeerId id) const
    {
        LOCK(cs_nodes_);
        for (auto it = list_.begin(); it != list_.end(); ++it)
            if ((*it)->id() == id)
                return it;
        return list_.end();
    }
    
    void AddNode(std::shared_ptr<Node> node)
    {
        LOCK(cs_nodes_);
        list_.push_back(node);
    }
    
    void EraseNode(std::shared_ptr<Node> node)
    {
        LOCK(cs_nodes_);
        auto it = std::find(list_.begin(), list_.end(), node);
        if (it != list_.end())
            list_.erase(it);
    }
    
    void EraseNode(PeerId id)
    {
        LOCK(cs_nodes_);
        for (auto it = list_.begin(); it != list_.end(); ++it)
            if ((*it)->id() == id)
                list_.erase(it);
    }
    
    void ClearDisconnected();
    
    void DisconnectBanNode(const SubNet& subnet);
    //bool AttemptToEvictConnection();
    int CountInbound();
    int CountOutbound();

    /*const std::list<Node*>& list() const
    {
        return list_;
    }*/
    
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


#endif // BTCLITE_NODE_H
