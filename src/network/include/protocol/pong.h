#ifndef BTCLITE_PROTOCOL_PONG_H
#define BTCLITE_PROTOCOL_PONG_H


#include "message.h"
#include "protocol/version.h"


namespace btclite {
namespace network {
namespace protocol {

namespace private_pong {

class Pong : public MessageData {
public:
    Pong() = default;
    
    explicit Pong(uint64_t nonce)
        : nonce_(nonce) {}
    
    //-------------------------------------------------------------------------
    bool RecvHandler(std::shared_ptr<Node> src_node, const Params& params) const;
    
    std::string Command() const
    {
        return msg_command::kMsgPong;
    }
    
    bool IsValid() const
    {
        return (nonce_ != 0);
    }
    
    void Clear()
    {
        nonce_ = 0;
    }
    
    size_t SerializedSize() const
    {
        return sizeof(nonce_);
    }
    
    //-------------------------------------------------------------------------
    bool operator==(const Pong& b) const
    {
        return nonce_ == b.nonce_;
    }
    
    bool operator!=(const Pong& b) const
    {
        return !(*this == b);
    }
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const
    {
        util::Serializer<Stream> serializer(out);
        serializer.SerialWrite(nonce_);
    }
    
    template <typename Stream>
    void Deserialize(Stream& in)
    {
        util::Deserializer<Stream> deserializer(in);
        deserializer.SerialRead(&nonce_);
    }
    
    //-------------------------------------------------------------------------
    uint64_t nonce() const
    {
        return nonce_;
    }
    
    void set_nonce(uint64_t nonce)
    {
        nonce_ = nonce;
    }
    
private:
    uint64_t nonce_ = 0;
};

} // namespace private_pong

using Pong = crypto::Hashable<private_pong::Pong>;

} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_PONG_H
