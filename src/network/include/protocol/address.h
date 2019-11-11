#ifndef BTCLITE_PROTOCOL_ADDRESS_H
#define BTCLITE_PROTOCOL_ADDRESS_H


#include "message.h"
#include "network_address.h"


namespace btclite {
namespace network {
namespace protocol {

class Addr : public MessageData {
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
    
    void set_addr_list(const List& addr_list)
    {
        addr_list_ = addr_list;
    }
    
    void set_addr_list(List&& addr_list) noexcept
    {
        addr_list_ = std::move(addr_list);
    }
    
private:
    List addr_list_;
};

} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_ADDRESS_H
