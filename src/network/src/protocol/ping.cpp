#include "protocol/ping.h"

#include "msg_process.h"
#include "protocol/pong.h"


namespace btclite {
namespace network {
namespace protocol {

Ping::Ping(uint64_t nonce)
    : nonce_(nonce) 
{
}
    
Ping::Ping(uint64_t nonce, uint32_t protocol_version)
    : nonce_(nonce), protocol_version_(protocol_version) 
{
}

bool Ping::RecvHandler(std::shared_ptr<Node> src_node, uint32_t magic) const
{
    if (src_node->protocol().version >= kBip31Version)
    {
        // Echo the message back with the nonce. This allows for two useful features:
        //
        // 1) A remote node can quickly check if the connection is operational
        // 2) Remote nodes can measure the latency of the network thread. If this node
        //    is overloaded it won't respond to pings quickly and the remote node can
        //    avoid sending us more work, like chain download requests.
        //
        // The nonce stops the remote getting confused between different pings: without
        // it, if the remote node sends a ping once per second and this node takes 5
        // seconds to respond to each, the 5th ping the remote sends would appear to
        // return very quickly.
        Pong pong(nonce_);
        SendMsg(pong, magic, src_node);
    }
    
    return true;
}

std::string Ping::Command() const
{
    return msg_command::kMsgPing;
}

bool Ping::IsValid() const
{
    if (protocol_version_ < kBip31Version)
        return true;
    return (nonce_ != 0);
}

void Ping::Clear()
{
    nonce_ = 0;
}

size_t Ping::SerializedSize() const
{
    if (protocol_version_ < kBip31Version)
        return 0;
    return sizeof(nonce_);
}

util::Hash256 Ping::GetHash() const
{
    return crypto::GetHash(*this);
}

bool Ping::operator==(const Ping& b) const
{
    return nonce_ == b.nonce_;
}

bool Ping::operator!=(const Ping& b) const
{
    return !(*this == b);
}

uint64_t Ping::nonce() const
{
    return nonce_;
}

void Ping::set_nonce(uint64_t nonce)
{
    nonce_ = nonce;
}

uint32_t Ping::protocol_version() const
{
    return protocol_version_;
}

void Ping::set_protocol_version(uint32_t version)
{
    protocol_version_ = version;
}

} // namespace protocol
} // namespace network
} // namespace btclite
