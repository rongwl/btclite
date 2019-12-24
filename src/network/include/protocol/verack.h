#ifndef BTCLITE_PROTOCOL_VERACK_H
#define BTCLITE_PROTOCOL_VERACK_H


#include "message.h"


namespace btclite {
namespace network {
namespace protocol {

namespace private_verack {

class Verack : public MessageData {
public:
    bool RecvHandler(std::shared_ptr<Node> src_node) const;
    
    std::string Command() const
    {
        return kMsgVerack;
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

} // namespace private_verack

using Verack = Hashable<private_verack::Verack>;

} // namespace protocol
} // namespace network
} // namespace btclite


#endif // BTCLITE_PROTOCOL_VERACK_H
