#ifndef BTCLITE_PROTOCOL_ADDRESS_H
#define BTCLITE_PROTOCOL_ADDRESS_H


#include "message.h"
#include "network_address.h"


namespace btclite {
namespace network {
namespace protocol {

class address : public MessageData {
public:
    using List = std::vector<btclite::network::NetAddr>;
    
    //-------------------------------------------------------------------------
    bool RecvHandler(std::shared_ptr<Node> src_node) const;
    
    std::string Command() const
    {
        return kMsgAddr;
    }
    
    bool IsValid() const
    {
        return !addr_list_.empty();
    }
    
    void Clear()
    {
        addr_list_.clear();
    }
    
    size_t SerializedSize() const;
    
    //-------------------------------------------------------------------------
    bool operator==(const address& b) const
    {
        return (addr_list_ == b.addr_list_);
    }
    
    bool operator!=(const address& b) const
    {
        return !(*this == b);
    }
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const
    {
        Serializer<Stream> serializer(out);
        serializer.SerialWrite(addr_list_);
    }
    
    template <typename Stream>
    void Deserialize(Stream& in)
    {
        Deserializer<Stream> deserializer(in);
        deserializer.SerialRead(&addr_list_);
    }
    
    //-------------------------------------------------------------------------
    const List& addr_list() const
    {
        return addr_list_;
    }
    
    List *mutable_addr_list()
    {
        return &addr_list_;
    }
    
private:
    List addr_list_;
};

using Addr = Hashable<address>;

} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_ADDRESS_H
