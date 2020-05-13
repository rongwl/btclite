#include "protocol/inventory.h"


namespace btclite {
namespace network {
namespace protocol {

bool Inv::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
}

size_t Inv::SerializedSize() const
{
    size_t size = util::VarIntSize(inv_vects_.size());
    for (const auto& inv_vect : inv_vects_)
        size += inv_vect.SerializedSize();
    return size;
}

} // namespace protocol
} // namespace network
} // namespace btclite
