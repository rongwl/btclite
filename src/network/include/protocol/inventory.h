#ifndef BTCLITE_PROTOCOL_INVENTORY_H
#define BTCLITE_PROTOCOL_INVENTORY_H


#include "message.h"
#include "inventory_vector.h"


namespace btclite {
namespace network {
namespace protocol {

class inventory : public MessageData {
public:
    bool RecvHandler(std::shared_ptr<Node> src_node) const;
    
    std::string Command() const
    {
        return kMsgInv;
    }
    
    size_t SerializedSize() const;
    
    bool IsValid() const
    {
        return !inv_vects_.empty();
    }
    
    void Clear()
    {
        inv_vects_.clear();
    }
    
    //-------------------------------------------------------------------------
    bool operator==(const inventory& b) const
    {
        return (inv_vects_ == b.inv_vects_);
    }
    
    bool operator!=(const inventory& b) const
    {
        return !(*this == b);
    }
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const
    {
        Serializer<Stream> serializer(out);
        serializer.SerialWrite(inv_vects_);
    }
    
    template <typename Stream>
    void Deserialize(Stream& in)
    {
        Deserializer<Stream> deserializer(in);
        deserializer.SerialRead(&inv_vects_);
    }
    
    //-------------------------------------------------------------------------
    const std::vector<InvVect>& inv_vects() const
    {
        return inv_vects_;
    }
    
    std::vector<InvVect> *mutable_inv_vects()
    {
        return &inv_vects_;
    }
    
private:
    std::vector<InvVect> inv_vects_;
};

using Inv = Hashable<inventory>;

} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_INVENTORY_H
