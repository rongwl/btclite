#include "message_types/version.h"

namespace btc_message {

const std::string Version::kCommand = "version";

bool Version::IsValid()
{
    return (value_ != 0) ||
           (services_ != 0) ||
           (timestamp_ != 0) ||
           (address_receiver_.IsValid()) ||
           (address_from_.IsValid()) ||
           (nonce_ != 0) ||
           (user_agent_.empty()) ||
           (start_height_ != 0) ||
           (relay_ != false);
}

void Version::Clear()
{
    value_ = 0;
    services_ = 0;
    timestamp_ = 0;
    address_receiver_.Clear();
    address_from_.Clear();
    nonce_ = 0;
    user_agent_.clear();
    user_agent_.shrink_to_fit();
    start_height_ = 0;
    relay_ = false;
}

Version& Version::operator=(const Version& b)
{
    value_ = b.value_;
    services_ = b.services_;
    timestamp_ = b.timestamp_;
    address_receiver_ = b.address_receiver_;
    address_from_ = b.address_from_;
    nonce_ = b.nonce_;
    user_agent_ = b.user_agent_;
    start_height_ = b.start_height_;
    relay_ = b.relay_;
    return *this;
}

Version& Version::operator=(Version&& b) noexcept
{
    value_ = b.value_;
    services_ = b.services_;
    timestamp_ = b.timestamp_;
    address_receiver_ = std::move(b.address_receiver_);
    address_from_ = std::move(b.address_from_);
    nonce_ = b.nonce_;
    user_agent_ = b.user_agent_;
    start_height_ = b.start_height_;
    relay_ = b.relay_;
    return *this;
}

void Version::ReadRawData(const uint8_t *in)
{

}

void Version::WriteRawData(VecWStream *out)
{

}

} // namespace btc_message
