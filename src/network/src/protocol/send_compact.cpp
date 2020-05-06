#include "protocol/send_compact.h"


namespace btclite {
namespace network {
namespace protocol {

namespace private_sendcmpct {

bool SendCmpct::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
}

} // private_sendcmpct

} // namespace protocol
} // namespace network
} // namespace btclite
