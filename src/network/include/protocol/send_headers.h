#ifndef BTCLITE_PROTOCOL_SEND_HEADERS_H
#define BTCLITE_PROTOCOL_SEND_HEADERS_H


#include "message.h"


namespace btclite {
namespace network {
namespace protocol {

class SendHeaders {
public:
    bool RecvHandler(std::shared_ptr<Node> src_node) const;    
    std::string Command() const;    
    bool IsValid() const;    
    void Clear();
    size_t SerializedSize() const;
    util::Hash256 GetHash() const;
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const {}
    template <typename Stream>
    void Deserialize(Stream& in) {}
};

} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_SEND_HEADERS_H
