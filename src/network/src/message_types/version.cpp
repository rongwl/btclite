#include "message_types/version.h"


namespace btclite {
namespace network {
namespace message_types{

const std::string Version::kCommand = kMsgVersion;

bool Version::IsValid() const
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


} // namespace message_types
} // namespace network
} // namespace btclite
