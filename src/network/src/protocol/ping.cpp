#include "protocol/ping.h"

#include "msg_process.h"
#include "net.h"
#include "protocol/pong.h"


namespace btclite {
namespace network {
namespace protocol {

bool Ping::RecvHandler(std::shared_ptr<Node> src_node, uint32_t magic) const
{
    if (src_node->protocol().version() >= kBip31Version)
    {
        // Echo the message back with the nonce. This allows for two useful features:
        //
        // 1) A remote node can quickly check if the connection is operational
        // 2) Remote nodes can measure the latency of the network thread. If this node
        //    is overloaded it won't respond to pings quickly and the remote node can
        //    avoid sending us more work, like chain download requests.
        //
        // The nonce stops the remote getting confused between different pings: without
        // it, if the remote node sends a ping once per second and this node takes 5
        // seconds to respond to each, the 5th ping the remote sends would appear to
        // return very quickly.
        Pong pong(nonce_);
        SendMsg(pong, magic, src_node);
    }
    
    return true;
}

} // namespace protocol
} // namespace network
} // namespace btclite
