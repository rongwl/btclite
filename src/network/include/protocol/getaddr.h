#ifndef BTCLITE_PROTOCOL_GETADDR_H
#define BTCLITE_PROTOCOL_GETADDR_H


#include "message.h"


namespace btclite {
namespace network {
namespace protocol {

namespace private_getaddress {

class GetAddress : public MessageData {
public:
    bool RecvHandler(std::shared_ptr<Node> src_node, const Params& params) const;
    
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
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const {}
    template <typename Stream>
    void Deserialize(Stream& in) {}
};

} // namespace private_getaddress

using GetAddr = crypto::Hashable<private_getaddress::GetAddress>;

} // namespace protocol
} // namespace network
} // namespace btclite

#endif //BTCLITE_PROTOCOL_GETADDR_H
