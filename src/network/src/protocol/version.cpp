#include "protocol/version.h"


namespace btclite {
namespace network {
namespace protocol{

const std::string VersionBase::kCommand = kMsgVersion;

VersionBase::VersionBase(const uint8_t *raw, size_t size)
    : VersionBase()
{
    std::vector<uint8_t> vec;
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    
    vec.reserve(size);
    vec.assign(raw, raw + size);
    Deserialize(byte_source);
}

bool VersionBase::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
}

bool VersionBase::IsValid() const
{
    return (version_ != 0) ||
           (services_ != 0) ||
           (timestamp_ != 0) ||
           (addr_recv_.IsValid()) ||
           (addr_from_.IsValid()) ||
           (nonce_ != 0) ||
           (user_agent_.empty()) ||
           (start_height_ != 0) ||
           (relay_ != false);
}

void VersionBase::Clear()
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

VersionBase& VersionBase::operator=(const VersionBase& b)
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

size_t VersionBase::SerializedSize() const
{
    return sizeof(version_) + sizeof(services_) + sizeof(timestamp_) +
           addr_recv_.SerializedSize() + addr_from_.SerializedSize() +
           sizeof(nonce_) + btclite::util::serialize::VarIntSize(user_agent_.size()) +
           user_agent_.size() + sizeof(start_height_) + sizeof(uint8_t);
}

VersionBase& VersionBase::operator=(VersionBase&& b) noexcept
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

bool VersionBase::operator==(const VersionBase& b) const
{
    return (version_ == b.version_) && 
           (services_ == b.services_) &&
           (timestamp_ == b.timestamp_) &&
           (addr_recv_ == b.addr_recv_) &&
           (addr_from_ == b.addr_from_) &&
           (nonce_ == b.nonce_) && 
           (user_agent_ == b.user_agent_) &&
           (start_height_ == b.start_height_) &&
           (relay_ == b.relay_);
}

bool VersionBase::operator!=(const VersionBase& b) const
{
    return !(*this == b);
}


} // namespace protocol
} // namespace network
} // namespace btclite
