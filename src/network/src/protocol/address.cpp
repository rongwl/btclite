#include "protocol/address.h"


namespace btclite {
namespace network {
namespace protocol {

bool Addr::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
}

size_t Addr::SerializedSize() const
{
    size_t size = 0;
    
    for (const auto& addr : addr_list_)
        size += addr.SerializedSize();
    
    return btclite::utility::serialize::VarIntSize(addr_list_.size()) + size;
}

} // namespace protocol
} // namespace network
} // namespace btclite
