#include "protocol/getaddr.h"


namespace btclite {
namespace network {
namespace protocol {

namespace private_getaddress {

bool GetAddress::RecvHandler(std::shared_ptr<Node> src_node, const Params& params) const
{
    return true;
}

} // namespace private_getaddress

} // namespace protocol
} // namespace network
} // namespace btclite
