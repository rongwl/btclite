#ifndef BTCLITE_PROTOCOL_SEND_HEADERS_H
#define BTCLITE_PROTOCOL_SEND_HEADERS_H


#include "message.h"


namespace btclite {
namespace network {
namespace protocol {

class sendheaders : public MessageData {
public:
    bool RecvHandler(std::shared_ptr<Node> src_node) const;
    
    std::string Command() const
    {
        return kMsgSendHeaders;
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

using SendHeaders = Hashable<sendheaders>;

} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_SEND_HEADERS_H
