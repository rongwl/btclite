#include "protocol/ping.h"


namespace btclite {
namespace network {
namespace protocol {

bool Ping::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
}

} // namespace protocol
} // namespace network
} // namespace btclite