#include "protocol/version.h"

#include "msg_process.h"
#include "net.h"
#include "peers.h"
#include "protocol/getaddr.h"
#include "protocol/reject.h"
#include "protocol/verack.h"


namespace btclite {
namespace network {
namespace protocol{

bool Version::RecvHandler(std::shared_ptr<Node> src_node, 
                          uint32_t magic, bool advertise_local) const
{
    Verack verack;
    
    // Each connection can only send one version message
    if (src_node->protocol().version() != 0) {
        src_node->mutable_misbehavior()->Misbehaving(src_node->id(), 1);
        return false;
    }
    
    if (!src_node->connection().is_inbound()) {
        SingletonPeers::GetInstance().SetServices(src_node->connection().addr(), 
                                                  services_);
    }
    if (!src_node->connection().is_inbound() && !src_node->connection().manual() 
            && !IsServiceFlagDesirable(services_)) {
        BTCLOG(LOG_LEVEL_INFO) << "Disconnecting peer " << src_node->id() 
                               << " for not offering the expected services ("
                               << std::showbase << std::hex << services_
                               << " offered, " << kDesirableServiceFlags
                               << "  expected).";
        std::stringstream ss;
        ss << "Expected to offer services " << std::showbase << std::hex 
           << kDesirableServiceFlags;
        DisconnectNode(src_node);
        return false;
    }
    
    if (services_ & ((1 << 7) | (1 << 5))) {
        if (util::GetTimeSeconds() < 1533096000) {
            // Immediately disconnect peers that use service bits 6 or 8 until August 1st, 2018
            // These bits have been used as a flag to indicate that a node is running incompatible
            // consensus rules instead of changing the network magic, so we're stuck disconnecting
            // based on these service bits, at least for a while.
            DisconnectNode(src_node);
            return false;
        }
    }
    
    if (protocol_version_ < kMinPeerProtoVersion) {
        BTCLOG(LOG_LEVEL_INFO) << "Disconnecting peer " << src_node->id()
                               << " for using obsolete version " 
                               << protocol_version_  << '.';
        std::stringstream ss;
        ss << "Version must be " << kMinPeerProtoVersion << " or greater";
        DisconnectNode(src_node);
        return false;
    }
    
    if (src_node->connection().is_inbound() && 
            !SingletonNodes::GetInstance().CheckIncomingNonce(nonce_)) {
        BTCLOG(LOG_LEVEL_INFO) << "Disconnecting peer " << src_node->id()
                               << " for connecting to self at " 
                               << src_node->connection().addr().ToString();
        DisconnectNode(src_node);
        return true;
    }
    
    if (src_node->connection().is_inbound()) {
        SendVersion(src_node, magic);
    }
    
    SendMsg(verack, magic, src_node);
    
    src_node->mutable_protocol()->set_services(services_);
    src_node->mutable_connection()->set_local_addr(addr_recv_);
    src_node->mutable_protocol()->set_start_height(start_height_);
    src_node->mutable_filter()->set_relay_txes(relay_);
    src_node->mutable_protocol()->set_version(protocol_version_);
    
    if (services_ & kNodeWitness) {
        src_node->mutable_relay_state()->is_witness = true;
    }
    
    if (!src_node->connection().is_inbound()) {
        // Advertise our address
        if (advertise_local && !IsInitialBlockDownload()) {
            SingletonLocalService::GetInstance().AdvertiseLocalAddr(src_node, true);
        }
        
        // Get recent addresses
        if (src_node->protocol().version() >= ProtocolVersion::kAddrTimeVersion || 
                SingletonPeers::GetInstance().Size() < 1000) {
            GetAddr getaddr;
            SendMsg(getaddr, magic, src_node);
            src_node->mutable_flooding_addrs()->set_sent_getaddr(true);
        }
        SingletonPeers::GetInstance().MakeTried(src_node->connection().addr());
    }
    
    BTCLOG(LOG_LEVEL_INFO) << "Receive version message: version=" << protocol_version_
                           << ", start_height=" << start_height_
                           << ", addr_me=" << addr_recv_.ToString()
                           << ", peer=" << src_node->id();
    
    util::AddTimeData(src_node->connection().addr(), 
            timestamp_ - util::GetTimeSeconds());
    
    return true;
}

bool Version::IsValid() const
{
    return (protocol_version_ != kUnknownProtoVersion &&
            services_ != 0 &&
            timestamp_ != 0 &&
            addr_recv_.IsValid() &&
            nonce_ != 0); 
}

void Version::Clear()
{
    protocol_version_ = kUnknownProtoVersion;
    services_ = kNodeNone;
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
    protocol_version_ = b.protocol_version_;
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
    size_t size = sizeof(protocol_version_) + sizeof(services_) + 
                  sizeof(timestamp_) + addr_recv_.SerializedSize() + 
                  addr_from_.SerializedSize() + sizeof(nonce_) + 
                  util::VarIntSize(user_agent_.size()) +
                  user_agent_.size() + sizeof(start_height_);
    if (protocol_version_ >= kRelayedTxsVersion)
        size += sizeof(bool);
    
    return size;
}

Version& Version::operator=(Version&& b) noexcept
{
    protocol_version_ = b.protocol_version_;
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
    return (protocol_version_ == b.protocol_version_) && 
           (services_ == b.services_) &&
           (timestamp_ == b.timestamp_) &&
           (addr_recv_.timestamp() == b.addr_recv_.timestamp()) &&
           (addr_recv_.services() == b.addr_recv_.services()) &&
           (addr_recv_ == b.addr_recv_) &&
           (addr_from_.timestamp() == b.addr_from_.timestamp()) &&
           (addr_from_.services() == b.addr_from_.services()) &&
           (addr_from_ == b.addr_from_) &&
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
