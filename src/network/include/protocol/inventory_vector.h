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
    
    InvVect(DataMsgType type, const util::Hash256& hash);
    
    //-------------------------------------------------------------------------
    size_t SerializedSize() const;
    void Clear();
    
    //-------------------------------------------------------------------------
    bool operator==(const InvVect& b) const;
    bool operator!=(const InvVect& b) const;
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const
    {
        util::Serializer<Stream> serializer(out);
        serializer.SerialWrite(type_);
        serializer.SerialWrite(hash_);
    }
    
    template <typename Stream>
    void Deserialize(Stream& in)
    {
        util::Deserializer<Stream> deserializer(in);
        deserializer.SerialRead(&type_);
        deserializer.SerialRead(&hash_);
    }
    
    //-------------------------------------------------------------------------
    DataMsgType type() const;    
    const util::Hash256& hash() const;
    
private:
    DataMsgType type_ = kUndefined;
    util::Hash256 hash_;
};

} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_INVENTORY_VECTOR_H
