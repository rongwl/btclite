#include "protocol/getblocks.h"


namespace btclite {
namespace network {
namespace protocol {

bool GetBlocks::RecvHandler(std::shared_ptr<Node> src_node, const Params& params) const
{
    return true;
}

} // namespace protocol
} // namespace network
} // namespace btclite
