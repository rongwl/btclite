#include "protocol/inventory_vector.h"


namespace btclite {
namespace network {
namespace protocol {

InvVect::InvVect(DataMsgType type, const util::Hash256& hash)
    : type_(type), hash_(hash) 
{
}

size_t InvVect::SerializedSize() const
{
    return sizeof(type_) + hash_.size();
}

void InvVect::Clear()
{
    type_ = kUndefined;
    hash_.fill(0);
}

bool InvVect::operator==(const InvVect& b) const
{
    return (type_ == b.type_ && hash_ == b.hash_);
}

bool InvVect::operator!=(const InvVect& b) const
{
    return !(*this == b);
}

DataMsgType InvVect::type() const
{
    return type_;
}

const util::Hash256& InvVect::hash() const
{
    return hash_;
}

} // namespace protocol
} // namespace network
} // namespace btclite
