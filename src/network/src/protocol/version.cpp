#include "protocol/version.h"

#include "msg_process.h"
#include "net.h"
#include "peers.h"
#include "protocol/reject.h"
#include "protocol/verack.h"


namespace btclite {
namespace network {
namespace protocol{

using namespace btclite::network::msg_process;
using namespace btclite::network::block_sync;

bool Version::RecvHandler(std::shared_ptr<Node> src_node) const
{
    Verack verack;
    
    // Each connection can only send one version message
    if (src_node->version() != 0) {
        Reject reject(Command(), kRejectDuplicate, "Duplicate version message");
        SendMsg(reject, src_node);
        SingletonBlockSync::GetInstance().Misbehaving(src_node->id(), 1);
        return false;
    }
    
    if (!src_node->is_inbound()) {
        SingletonPeers::GetInstance().SetServices(src_node->addr(), services_);
    }
    if (!src_node->is_inbound() && !src_node->manual() 
            && !btclite::network::service_flags::IsDesirable(services_)) {
        BTCLOG(LOG_LEVEL_INFO) << "Disconnecting peer " << src_node->id() 
                               << " for not offering the expected services ("
                               << std::showbase << std::hex << services_
                               << " offered, " << kDesirableServiceFlags
                               << "  expected).";
        std::stringstream ss;
        ss << "Expected to offer services " << std::showbase << std::hex 
           << kDesirableServiceFlags;
        Reject reject(Command(), kRejectNonstandard, std::move(ss.str()));
        SendMsg(reject, src_node);
        src_node->set_disconnected(true);
        return false;
    }
    
    if (services_ & ((1 << 7) | (1 << 5))) {
        if (btclite::utility::util_time::GetTimeSeconds() < 1533096000) {
            // Immediately disconnect peers that use service bits 6 or 8 until August 1st, 2018
            // These bits have been used as a flag to indicate that a node is running incompatible
            // consensus rules instead of changing the network magic, so we're stuck disconnecting
            // based on these service bits, at least for a while.
            src_node->set_disconnected(true);
            return false;
        }
    }
    
    if (version_ < kMinPeerProtoVersion) {
        BTCLOG(LOG_LEVEL_INFO) << "Disconnecting peer " << src_node->id()
                               << " for using obsolete version " << version_  << '.';
        std::stringstream ss;
        ss << "Version must be " << kMinPeerProtoVersion << " or greater";
        Reject reject(Command(), kRejectObsolete, std::move(ss.str()));
        SendMsg(reject, src_node);
        src_node->set_disconnected(true);
        return false;
    }
    
    if (src_node->is_inbound() && !SingletonNodes::GetInstance().CheckIncomingNonce(nonce_))
    {
        BTCLOG(LOG_LEVEL_INFO) << "Disconnecting peer " << src_node->id()
                               << " for connecting to self at " << src_node->addr().ToString();
        src_node->set_disconnected(true);
        return true;
    }
    
    if (src_node->is_inbound())
        SendVersion(src_node);
    
    SendMsg(verack, src_node);
    
    src_node->set_services(static_cast<ServiceFlags>(services_));
    src_node->set_local_addr(addr_recv_);
    src_node->set_start_height(start_height_);
    src_node->mutable_filter()->set_relay_txes(relay_);
    src_node->set_version(version_);
    
    if (services_ & kNodeWitness)
        SingletonBlockSync::GetInstance().SetIsWitness(src_node->id(), true);
    
    UpdatePreferredDownload(src_node);
    
    if (!src_node->is_inbound()) {
        // Advertise our address
        if (SingletonNetArgs::GetInstance().listening() && !IsInitialBlockDownload()) {
        
        }
    }
    
    return true;
}

bool Version::IsValid() const
{
    return (version_ != 0 &&
            services_ != 0 &&
            timestamp_ != 0 &&
            addr_recv_.IsValid() &&
            nonce_ != 0); 
}

void Version::Clear()
{
    version_ = 0;
    services_ = 0;
    timestamp_ = 0;
    addr_recv_.Clear();
    addr_from_.Clear();
    nonce_ = 0;
    user_agent_.clear();
    user_agent_.shrink_to_fit();
    start_height_ = 0;
    relay_ = false;
}

Version& Version::operator=(const Version& b)
{
    version_ = b.version_;
    services_ = b.services_;
    timestamp_ = b.timestamp_;
    addr_recv_ = b.addr_recv_;
    addr_from_ = b.addr_from_;
    nonce_ = b.nonce_;
    user_agent_ = b.user_agent_;
    start_height_ = b.start_height_;
    relay_ = b.relay_;
    return *this;
}

size_t Version::SerializedSize() const
{
    return sizeof(version_) + sizeof(services_) + sizeof(timestamp_) +
           addr_recv_.SerializedSize() + addr_from_.SerializedSize() +
           sizeof(nonce_) + btclite::utility::serialize::VarIntSize(user_agent_.size()) +
           user_agent_.size() + sizeof(start_height_) + sizeof(uint8_t);
}

Version& Version::operator=(Version&& b) noexcept
{
    version_ = b.version_;
    services_ = b.services_;
    timestamp_ = b.timestamp_;
    addr_recv_ = std::move(b.addr_recv_);
    addr_from_ = std::move(b.addr_from_);
    nonce_ = b.nonce_;
    user_agent_ = b.user_agent_;
    start_height_ = b.start_height_;
    relay_ = b.relay_;
    return *this;
}

bool Version::operator==(const Version& b) const
{
    return (version_ == b.version_) && 
           (services_ == b.services_) &&
           (timestamp_ == b.timestamp_) &&
           (addr_recv_.proto_addr().timestamp() == b.addr_recv_.proto_addr().timestamp()) &&
           (addr_recv_.proto_addr().services() == b.addr_recv_.proto_addr().services()) &&
           !std::memcmp(addr_recv_.proto_addr().ip().begin(), b.addr_recv_.proto_addr().ip().begin(), kIpByteSize) &&
           (addr_recv_.proto_addr().port() == b.addr_recv_.proto_addr().port()) &&
           (addr_from_.proto_addr().timestamp() == b.addr_from_.proto_addr().timestamp()) &&
           (addr_from_.proto_addr().services() == b.addr_from_.proto_addr().services()) &&
           !std::memcmp(addr_from_.proto_addr().ip().begin(), b.addr_from_.proto_addr().ip().begin(), kIpByteSize) &&
           (addr_from_.proto_addr().port() == b.addr_from_.proto_addr().port()) &&
           (nonce_ == b.nonce_) && 
           (user_agent_ == b.user_agent_) &&
           (start_height_ == b.start_height_) &&
           (relay_ == b.relay_);
}

bool Version::operator!=(const Version& b) const
{
    return !(*this == b);
}


} // namespace protocol
} // namespace network
} // namespace btclite
