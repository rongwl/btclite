#include "protocol/getheaders.h"


namespace btclite {
namespace network {
namespace protocol {

bool GetHeaders::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
}

} // namespace protocol
} // namespace network
} // namespace btclite
