#ifndef BTCLITE_PROTOCOL_GETADDR_H
#define BTCLITE_PROTOCOL_GETADDR_H


#include "message.h"


namespace btclite {
namespace network {
namespace protocol {

class getaddress : public MessageData {
public:
    bool RecvHandler(std::shared_ptr<Node> src_node) const;
    
    std::string Command() const
    {
        return kMsgGetAddr;
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
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const {}
    template <typename Stream>
    void Deserialize(Stream& in) {}
};

using GetAddr = Hashable<getaddress>;

} // namespace protocol
} // namespace network
} // namespace btclite

#endif //BTCLITE_PROTOCOL_GETADDR_H