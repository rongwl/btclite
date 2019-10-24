#include "protocol/version.h"


namespace btclite {
namespace network {
namespace protocol{

bool Version::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
}

bool Version::IsValid() const
{
    return (version_ >= kMinPeerProtoVersion &&
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