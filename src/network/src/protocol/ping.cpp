#include "protocol/ping.h"


namespace btclite {
namespace network {
namespace protocol {

const std::string Ping::kCommand = kMsgPing;

Ping::Ping(const uint8_t *raw)
{
    std::vector<uint8_t> vec;
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    size_t size = sizeof(nonce_);
    
    vec.reserve(size);
    vec.assign(raw, raw + size);
    Deserialize(byte_source);
}

bool Ping::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
}

} // namespace protocol
} // namespace network
} // namespace btclite
