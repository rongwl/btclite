#include "protocol/send_compact.h"


namespace btclite {
namespace network {
namespace protocol {

SendCmpct::SendCmpct(bool high_bandwidth_mode, uint64_t version)
    : high_bandwidth_mode_(high_bandwidth_mode), version_(version)
{
}

bool SendCmpct::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
}

std::string SendCmpct::Command() const
{
    return msg_command::kMsgSendCmpct;
}

bool SendCmpct::IsValid() const
{
    return (version_ == 1 || version_ == 2);
}

void SendCmpct::Clear() 
{
    high_bandwidth_mode_ = false;
    version_ = 0;
}

size_t SendCmpct::SerializedSize() const
{
    return sizeof(high_bandwidth_mode_) + sizeof(version_);
}

util::Hash256 SendCmpct::GetHash() const
{
    return crypto::GetHash(*this);
}

bool SendCmpct::operator==(const SendCmpct& b) const
{
    return (high_bandwidth_mode_ == b.high_bandwidth_mode_ &&
            version_ == b.version_);
}

bool SendCmpct::operator!=(const SendCmpct &b) const
{
    return !(*this == b);
}

bool SendCmpct::high_bandwidth_mode() const
{
    return high_bandwidth_mode_;
}

void SendCmpct::set_high_bandwidth_mode(bool high_bandwidth_mode)
{
    high_bandwidth_mode_ = high_bandwidth_mode;
}

uint64_t SendCmpct::version() const
{
    return version_;
}

void SendCmpct::set_version(uint64_t version)
{
    version_ = version;
}

} // namespace protocol
} // namespace network
} // namespace btclite
