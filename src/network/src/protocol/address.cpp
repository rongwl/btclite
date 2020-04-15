#include "protocol/address.h"


namespace btclite {
namespace network {
namespace protocol {

namespace private_address {

bool Address::RecvHandler(std::shared_ptr<Node> src_node, const Params& params) const
{
    return true;
}

size_t Address::SerializedSize() const
{
    size_t size = util::VarIntSize(addr_list_.size());
    
    for (const auto& addr : addr_list_)
        size += addr.SerializedSize();
    
    return size;
}

} // namespace private_address

} // namespace protocol
} // namespace network
} // namespace btclite
