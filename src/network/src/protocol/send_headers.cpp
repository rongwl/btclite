#include "protocol/send_headers.h"


namespace btclite {
namespace network {
namespace protocol {

namespace private_sendheaders {

bool SendHeaders::RecvHandler(std::shared_ptr<Node> src_node, const Params& params) const
{
    return true;
}

} // namespace private_sendheaders

} // namespace protocol
} // namespace network
} // namespace btclite
