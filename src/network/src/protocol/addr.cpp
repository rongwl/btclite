#include "protocol/addr.h"


namespace btclite {
namespace network {
namespace protocol {

bool Addr::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
}

std::string Addr::Command() const
{
    return msg_command::kMsgAddr;
}

bool Addr::IsValid() const
{
    return !addr_list_.empty();
}

void Addr::Clear()
{
    addr_list_.clear();
}

size_t Addr::SerializedSize() const
{
    size_t size = util::VarIntSize(addr_list_.size());
    
    for (const auto& addr : addr_list_)
        size += addr.SerializedSize();
    
    return size;
}

util::Hash256 Addr::GetHash() const
{
    return crypto::GetHash(*this);
}

bool Addr::operator==(const Addr& b) const
{
    return (addr_list_ == b.addr_list_);
}

bool Addr::operator!=(const Addr& b) const
{
    return !(*this == b);
}

const Addr::List& Addr::addr_list() const
{
    return addr_list_;
}

Addr::List* Addr::mutable_addr_list()
{
    return &addr_list_;
}

} // namespace protocol
} // namespace network
} // namespace btclite
