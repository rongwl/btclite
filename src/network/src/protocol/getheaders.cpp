#include "protocol/getheaders.h"


namespace btclite {
namespace network {
namespace protocol {

bool GetHeaders::RecvHandler(std::shared_ptr<Node> src_node, const Params& params) const
{
    return true;
}

std::string GetHeaders::Command() const
{
    return msg_command::kMsgGetHeaders;
}

} // namespace protocol
} // namespace network
} // namespace btclite
