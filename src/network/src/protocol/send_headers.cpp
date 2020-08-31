#include "protocol/send_headers.h"


namespace btclite {
namespace network {
namespace protocol {

bool SendHeaders::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
}

std::string SendHeaders::Command() const
{
    return msg_command::kMsgSendHeaders;
}

bool SendHeaders::IsValid() const
{
    return true;
}

void SendHeaders::Clear() 
{
}

size_t SendHeaders::SerializedSize() const
{
    return 0;
}

util::Hash256 SendHeaders::GetHash() const
{
    return crypto::GetHash(*this);
}

} // namespace protocol
} // namespace network
} // namespace btclite
