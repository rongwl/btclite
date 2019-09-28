#ifndef BTCLITE_MESSAGE_TYPES_ADDRESS_H
#define BTCLITE_MESSAGE_TYPES_ADDRESS_H


#include "blob.h"
#include "constants.h"


namespace btclite {
namespace network {
namespace message_types{

using IpAddr = Bytes<kIpByteSize>;

struct NetAddr {  
    NetAddr()
        : timestamp(0), services(0), ip(), port(0) {}
    
    //-------------------------------------------------------------------------
    bool IsValid() const;
    void Clear();
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const;
    template <typename Stream>
    void UnSerialize(Stream& in);
    
    //-------------------------------------------------------------------------
    uint32_t timestamp;
    uint64_t services;
    IpAddr ip;
    uint16_t port;
};

template <typename Stream>
void NetAddr::Serialize(Stream& out) const
{

}

template <typename Stream>
void NetAddr::UnSerialize(Stream& in)
{

}

} // namespace message_types
} // namespace network
} // namespace btclite

#endif // BTCLITE_MESSAGE_TYPES_ADDRESS_H
