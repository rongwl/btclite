#ifndef BTCLITE_MESSAGE_TYPES_H
#define BTCLITE_MESSAGE_TYPES_H


#include <cstdint>
#include <memory>
#include <string>

#include "hash.h"


//class Node;

namespace btc_message {

struct NetAddr {
    uint32_t time_;
    uint64_t serviecs_;
    char ip_[16];
    uint16_t port_;
};

template <size_t N>
struct VarInt {
    uint8_t value_[N];
};

template <size_t N, size_t (*F)(size_t)>
struct VarStr {
    VarInt<F(N)> length_;
    char string_[N];
};

} // namespace btc_message

class BaseMsgType {
public:
    virtual void Serialize(ByteSink<std::vector<uint8_t> >& out) const = 0;
    virtual void UnSerialize(const uint8_t *in) = 0;
    
    const std::string& command() const
    {
        return command_;
    }
    
protected:
    std::string command_;
};


#endif // BTCLITE_MESSAGE_TYPES_H
