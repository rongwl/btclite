#include "protocol/getaddr.h"


namespace btclite {
namespace network {
namespace protocol {

bool GetAddr::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
}

std::string GetAddr::Command() const
{
    return msg_command::kMsgGetAddr;
}

bool GetAddr::IsValid() const
{
    return true;
}

void GetAddr::Clear()
{
}

size_t GetAddr::SerializedSize() const
{
    return 0;
}

util::Hash256 GetAddr::GetHash() const
{
    return crypto::GetHash(*this);
}

} // namespace protocol
} // namespace network
} // namespace btclite
