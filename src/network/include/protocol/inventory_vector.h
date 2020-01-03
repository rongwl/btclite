#ifndef BTCLITE_PROTOCOL_INVENTORY_VECTOR_H
#define BTCLITE_PROTOCOL_INVENTORY_VECTOR_H


#include "hash.h"


namespace btclite {
namespace network {
namespace protocol {

/* getdata / inv message types.
 * These numbers are defined by the protocol. When adding a new value, be sure
 * to mention it in the respective BIP.
 */
enum DataMsgType : uint32_t {
    kUndefined = 0,
    kMsgTx = 1,
    kMsgBlock = 2,
    // the following can only occur in getdata. invs always use tx or block.
    kMsgFilteredBlock = 3,  // defined in bip37
    kMsgCmpctBlock = 4,     // defined in bip152
    kMsgWitnessFlag = 1 << 30,
    kMsgWitnessBlock = kMsgBlock | kMsgWitnessFlag, // defined in bip144
    kMsgWitnessTx = kMsgTx | kMsgWitnessFlag,       // defined in bip144
    kMsgFilteredWitnessBlock = kMsgFilteredBlock | kMsgWitnessFlag,
};

class InvVect {
public:
    InvVect() = default;
    
    InvVect(DataMsgType type, const Hash256& hash)
        : type_(type), hash_(hash) {}
    
    //-------------------------------------------------------------------------
    size_t SerializedSize() const
    {
        return sizeof(type_) + hash_.size();
    }
    
    void Clear()
    {
        type_ = kUndefined;
        hash_.Clear();
    }
    
    //-------------------------------------------------------------------------
    bool operator==(const InvVect& b) const
    {
        return (type_ == b.type_ && hash_ == b.hash_);
    }
    
    bool operator!=(const InvVect& b) const
    {
        return !(*this == b);
    }
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const
    {
        Serializer<Stream> serializer(out);
        serializer.SerialWrite(type_);
        serializer.SerialWrite(hash_);
    }
    
    template <typename Stream>
    void Deserialize(Stream& in)
    {
        Deserializer<Stream> deserializer(in);
        deserializer.SerialRead(&type_);
        deserializer.SerialRead(&hash_);
    }
    
    //-------------------------------------------------------------------------
    DataMsgType type() const
    {
        return type_;
    }
    
    const Hash256& hash() const
    {
        return hash_;
    }
    
private:
    DataMsgType type_ = kUndefined;
    Hash256 hash_;
};

} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_INVENTORY_VECTOR_H
