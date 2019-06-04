#include "node.h"


NodeState::NodeState(btclite::NetAddr& addr, std::string addr_name)
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
        if (it->disconnected()) {
        
        }
    }
}

void Nodes::ClearNodeState()
{

}
