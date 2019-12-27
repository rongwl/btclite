#include "protocol/ping.h"

#include "msg_process.h"
#include "net.h"
#include "protocol/pong.h"
#include "random.h"


namespace btclite {
namespace network {
namespace protocol {

namespace private_ping {

using namespace btclite::network::msg_process;

bool Ping::RecvHandler(std::shared_ptr<Node> src_node) const
{
    if (src_node->protocol_version() >= VersionCode::kBip31Version)
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
        SendMsg(pong, src_node);
    }
    
    return true;
}

void Ping::PingTimeoutCb(std::shared_ptr<Node> node)
{
    if (SingletonNetInterrupt::GetInstance())
        return;
    
    if (node->disconnected() || !node->conn_established()) {
        SingletonTimerMng::GetInstance().StopTimer(node->timers().ping_timer);
        return;
    }
    
    if (node->time().ping_time.ping_nonce_sent) {
        BTCLOG(LOG_LEVEL_WARNING) << "Peer " << node->id() << " ping timeout.";
        SingletonTimerMng::GetInstance().StopTimer(node->timers().ping_timer);
        node->set_disconnected(true);
        return;
    }
    
    // send ping
    btclite::network::protocol::Ping ping(btclite::utility::GetUint64());
    if (node->protocol_version() < VersionCode::kBip31Version)
        ping.set_protocol_version(0);
    SendMsg(ping, node);
}

} // namespace private_ping

} // namespace protocol
} // namespace network
} // namespace btclite
