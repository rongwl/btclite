#include "protocol/inventory.h"


namespace btclite {
namespace network {
namespace protocol {

namespace private_inventory {

bool Inventory::RecvHandler(std::shared_ptr<Node> src_node, const Params& params) const
{
    return true;
}

size_t Inventory::SerializedSize() const
{
    size_t size = util::VarIntSize(inv_vects_.size());
    for (const auto& inv_vect : inv_vects_)
        size += inv_vect.SerializedSize();
    return size;
}

} // private_inventory

} // namespace protocol
} // namespace network
} // namespace btclite
