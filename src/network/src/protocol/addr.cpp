#include "protocol/addr.h"


namespace btclite {
namespace network {
namespace protocol {

bool Addr::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
}

size_t Addr::SerializedSize() const
{
    size_t size = util::VarIntSize(addr_list_.size());
    
    for (const auto& addr : addr_list_)
        size += addr.SerializedSize();
    
    return size;
}

} // namespace protocol
} // namespace network
} // namespace btclite
