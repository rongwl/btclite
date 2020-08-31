#ifndef BTCLITE_PROTOCOL_ADDR_H
#define BTCLITE_PROTOCOL_ADDR_H


#include "message.h"
#include "network_address.h"


namespace btclite {
namespace network {
namespace protocol {

class Addr {
public:
    using List = std::vector<NetAddr>;
    
    //-------------------------------------------------------------------------
    bool RecvHandler(std::shared_ptr<Node> src_node) const;    
    std::string Command() const;    
    bool IsValid() const;    
    void Clear();    
    size_t SerializedSize() const;    
    util::Hash256 GetHash() const;
    
    //-------------------------------------------------------------------------
    bool operator==(const Addr& b) const;
    bool operator!=(const Addr& b) const;
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const
    {
        util::Serializer<Stream> serializer(out);
        serializer.SerialWrite(addr_list_);
    }
    
    template <typename Stream>
    void Deserialize(Stream& in)
    {
        util::Deserializer<Stream> deserializer(in);
        deserializer.SerialRead(&addr_list_);
    }
    
    //-------------------------------------------------------------------------
    const List& addr_list() const;    
    List *mutable_addr_list();
    
private:
    List addr_list_;
};


} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_ADDR_H
