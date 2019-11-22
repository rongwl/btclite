#include "protocol/pong.h"


namespace btclite {
namespace network {
namespace protocol {

bool pong::RecvHandler(std::shared_ptr<Node> src_node) const
{
    if (src_node->time().ping_time.ping_nonce_sent == 0) {
        BTCLOG(LOG_LEVEL_WARNING) << "Unsolicited pong without ping, peer=" 
                                  << src_node->id();
        return false;
    }
    
    if (src_node->time().ping_time.ping_nonce_sent != nonce_) {
        BTCLOG(LOG_LEVEL_WARNING) << "Received mismatch nonce pong: " 
                                  << std::hex << std::showbase
                                  << src_node->time().ping_time.ping_nonce_sent
                                  << " expected, " << nonce_ << " received, peer="
                                  << src_node->id();
        return false;
    }
    
    src_node->mutable_time()->ping_time.ping_nonce_sent = 0;
        
    return true;
}

} // namespace protocol
} // namespace network
} // namespace btclite
