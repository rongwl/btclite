#ifndef BTCLITE_PROTOCOL_SEND_HEADERS_H
#define BTCLITE_PROTOCOL_SEND_HEADERS_H


#include "message.h"


namespace btclite {
namespace network {
namespace protocol {

namespace private_sendheaders {

class SendHeaders {
public:
    bool RecvHandler(std::shared_ptr<Node> src_node) const;
    
    std::string Command() const
    {
        return msg_command::kMsgSendHeaders;
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

} // namespace private_sendheaders

using SendHeaders = crypto::Hashable<private_sendheaders::SendHeaders>;

} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_SEND_HEADERS_H
