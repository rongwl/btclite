#include "protocol/inventory.h"


namespace btclite {
namespace network {
namespace protocol {

bool Inv::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
}

std::string Inv::Command() const
{
    return msg_command::kMsgInv;
}

size_t Inv::SerializedSize() const
{
    size_t size = util::VarIntSize(inv_vects_.size());
    for (const auto& inv_vect : inv_vects_)
        size += inv_vect.SerializedSize();
    return size;
}

util::Hash256 Inv::GetHash() const
{
    return crypto::GetHash(*this);
}

bool Inv::IsValid() const
{
    return !inv_vects_.empty();
}

void Inv::Clear()
{
    inv_vects_.clear();
}

bool Inv::operator==(const Inv& b) const
{
    return (inv_vects_ == b.inv_vects_);
}

bool Inv::operator!=(const Inv& b) const
{
    return !(*this == b);
}

const std::vector<InvVect>& Inv::inv_vects() const
{
    return inv_vects_;
}

std::vector<InvVect>* Inv::mutable_inv_vects()
{
    return &inv_vects_;
}

} // namespace protocol
} // namespace network
} // namespace btclite
