#ifndef BTCLITE_MESSAGE_TYPES_ADDRESS_H
#define BTCLITE_MESSAGE_TYPES_ADDRESS_H


#include "blob.h"
#include "constants.h"
#include "network_address.h"
#include "node.h"
#include "serialize.h"


namespace btclite {
namespace network {
namespace protocol {

struct NetAddr {
    uint32_t timestamp;
    uint64_t services;
    IpAddr ip;
    uint16_t port;
    
    //-------------------------------------------------------------------------
    bool IsValid() const;
    void Clear();
    size_t SerializedSize() const
    {
        return sizeof(timestamp) + sizeof(services) + ip.size() + sizeof(port);
    }
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const;
    template <typename Stream>
    void Deserialize(Stream& in);
    
    //-------------------------------------------------------------------------
    bool operator==(const NetAddr& b) const;
    bool operator!=(const NetAddr& b) const;
};

template <typename Stream>
void NetAddr::Serialize(Stream& out) const
{
    Serializer<Stream> serializer(out);
    
    serializer.SerialWrite(timestamp);
    serializer.SerialWrite(services);
    serializer.SerialWrite(ip);
    serializer.SerialWrite(port);
}

template <typename Stream>
void NetAddr::Deserialize(Stream& in)
{
    Deserializer<Stream> deserializer(in);
    
    deserializer.SerialRead(&timestamp);
    deserializer.SerialRead(&services);
    deserializer.SerialRead(&ip);
    deserializer.SerialRead(&port);
}

} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_MESSAGE_TYPES_ADDRESS_H
