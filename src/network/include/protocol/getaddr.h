#ifndef BTCLITE_PROTOCOL_GETADDR_H
#define BTCLITE_PROTOCOL_GETADDR_H


#include "message.h"


namespace btclite {
namespace network {
namespace protocol {

class GetAddr {
public:
    bool RecvHandler(std::shared_ptr<Node> src_node) const;
    
    std::string Command() const
    {
        return msg_command::kMsgGetAddr;
    }
    
    bool IsValid() const
    {
        return true;
    }
    
    void Clear() {}
    
    size_t SerializedSize() const
    {
        return 0;
    }
    
    util::Hash256 GetHash() const
    {
        return crypto::GetHash(*this);
    }
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const {}
    template <typename Stream>
    void Deserialize(Stream& in) {}
};

} // namespace protocol
} // namespace network
} // namespace btclite

#endif //BTCLITE_PROTOCOL_GETADDR_H
