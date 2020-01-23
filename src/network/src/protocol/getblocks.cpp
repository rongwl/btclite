#include "protocol/getblocks.h"


namespace btclite {
namespace network {
namespace protocol {

namespace private_getblocks {

bool GetBlocks::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
}

} // namespace private_getblocks

} // namespace protocol
} // namespace network
} // namespace btclite
