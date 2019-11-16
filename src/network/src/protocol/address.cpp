#include "protocol/address.h"


namespace btclite {
namespace network {
namespace protocol {

bool address::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
}

size_t address::SerializedSize() const
{
    size_t size = btclite::utility::serialize::VarIntSize(addr_list_.size());
    
    for (const auto& addr : addr_list_)
        size += addr.SerializedSize();
    
    return size;
}

} // namespace protocol
} // namespace network
} // namespace btclite
