#include "msg_process.h"

#include <event2/buffer.h>


using namespace btclite::network::protocol;

namespace btclite {
namespace network {
namespace msgprocess{

MessageData *MsgDataFactory(const MessageHeader& header, const uint8_t *raw)
{
    if (header.command() == kMsgVersion) {
        return new Version(raw, header.payload_length());
    }
    
    return nullptr;
}

bool ParseMsg(std::shared_ptr<Node> src_node)
{
    struct evbuffer *buf;
    uint8_t *raw;
    
    if (!src_node->bev())
        return false;
    
    if (nullptr == (buf = bufferevent_get_input(src_node->mutable_bev())))
        return false;
    
    if (src_node->disconnected())
        return false;
    
    raw = evbuffer_pullup(buf, MessageHeader::kSize);
    while (raw) {
        MessageHeader header(raw);
        
        if (!header.IsValid()) {
            BTCLOG(LOG_LEVEL_ERROR) << "Received invalid message header from peer " << src_node->id();
            return false;
        }
        
        raw = evbuffer_pullup(buf, header.payload_length());
        MessageData *message = MsgDataFactory(header, raw);
        if (!message) {
            BTCLOG(LOG_LEVEL_ERROR) << "Prasing message data from peer " << src_node->id() << " failed.";
            return false;
        }
        
        message->RecvHandler(src_node);
        
        raw = evbuffer_pullup(buf, MessageHeader::kSize);
    }
    
    return true;
}

} // namespace msgprocess
} // namespace network
} // namespace btclite
