#include "protocol/pong.h"


namespace btclite {
namespace network {
namespace protocol {

Pong::Pong(uint64_t nonce)
    : nonce_(nonce) 
{
}

bool Pong::RecvHandler(std::shared_ptr<Node> src_node) const
{
    if (src_node->time().ping_time.ping_nonce_sent == 0) {
        BTCLOG(LOG_LEVEL_WARNING) << "Unsolicited pong without ping, peer=" 
                                  << src_node->id();
        return false;
    }
    
    if (src_node->time().ping_time.ping_nonce_sent != nonce_) {
        BTCLOG(LOG_LEVEL_WARNING) << "Received mismatch nonce pong: " 
                                  << std::hex << std::showbase
                                  << src_node->time().ping_time.ping_nonce_sent
                                  << " expected, " << nonce_ << " received, peer="
                                  << src_node->id();
        return false;
    }
    
    src_node->mutable_time()->ping_time.ping_nonce_sent = 0;
        
    return true;
}

std::string Pong::Command() const
{
    return msg_command::kMsgPong;
}

bool Pong::IsValid() const
{
    return (nonce_ != 0);
}

void Pong::Clear()
{
    nonce_ = 0;
}

size_t Pong::SerializedSize() const
{
    return sizeof(nonce_);
}

util::Hash256 Pong::GetHash() const
{
    return crypto::GetHash(*this);
}

bool Pong::operator==(const Pong& b) const
{
    return nonce_ == b.nonce_;
}

bool Pong::operator!=(const Pong& b) const
{
    return !(*this == b);
}

uint64_t Pong::nonce() const
{
    return nonce_;
}

void Pong::set_nonce(uint64_t nonce)
{
    nonce_ = nonce;
}

} // namespace protocol
} // namespace network
} // namespace btclite
