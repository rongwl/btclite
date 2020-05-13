#include "protocol/send_compact.h"


namespace btclite {
namespace network {
namespace protocol {

bool SendCmpct::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
}

} // namespace protocol
} // namespace network
} // namespace btclite
