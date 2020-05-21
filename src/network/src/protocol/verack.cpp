#include "protocol/verack.h"

#include "msg_process.h"
#include "net.h"
#include "protocol/ping.h"
#include "protocol/send_headers.h"
#include "protocol/send_compact.h"
#include "protocol/version.h"
#include "random.h"


namespace btclite {
namespace network {
namespace protocol {

bool Verack::RecvHandler(std::shared_ptr<Node> src_node, uint32_t magic, 
                         bool advertise_local, const LocalService& local_service) const
{
    if (src_node->protocol().version >= kSendheadersVersion) {
        // Tell our peer we prefer to receive headers rather than inv's
        // We send this to non-NODE NETWORK peers as well, because even
        // non-NODE NETWORK peers can announce blocks (such as pruning
        // nodes)
        SendHeaders send_headers;
        SendMsg(send_headers, magic, src_node);
    }
    if (src_node->protocol().version >= kShortIdsBlocksVersion) {
        // Tell our peer we are willing to provide version 1 or 2 cmpctblocks
        // However, we do not request new block announcements using
        // cmpctblock messages.
        // We send this to non-NODE NETWORK peers as well, because
        // they may wish to request compact blocks from us
        SendCmpct send_compact(false, 2);
        if (local_service.service() & ServiceFlags::kNodeWitness)
            SendMsg(send_compact, magic, src_node);
        send_compact.set_version(1);
        SendMsg(send_compact, magic, src_node);
    }
    src_node->mutable_connection()->set_connection_state(NodeConnection::kEstablished);
    
    // send ping
    Ping ping(util::RandUint64());
    if (src_node->protocol().version < kBip31Version)
        ping.set_protocol_version(0);
    SendMsg(ping, magic, src_node);
    // start ping timer
    src_node->mutable_timers()->ping_timer = 
        util::SingletonTimerMng::GetInstance().StartTimer(
            kPingInterval*1000, kPingInterval*1000, 
            std::bind(&Node::PingTimeoutCb, src_node, std::placeholders::_1),
            magic);
    
    // advertise local address
    if (advertise_local) {
        if (!IsInitialBlockDownload()) {
            local_service.AdvertiseLocalAddr(src_node, false);
        }
        src_node->mutable_timers()->advertise_local_addr_timer =
            util::SingletonTimerMng::GetInstance().StartTimer(
                IntervalNextSend(kAdvertiseLocalInterval)*1000, 0, 
                                 AdvertiseLocalTimeoutCb, 
                                 src_node, std::ref(local_service));
    }
    
    // relay flooding addresses
    src_node->mutable_timers()->broadcast_addrs_timer = 
        util::SingletonTimerMng::GetInstance().StartTimer(kRelayAddrsInterval*1000, 0, 
                                                          RelayFloodingAddrsTimeoutCb, 
                                                          src_node, magic);
    
    return true;
}


} // namespace protocol
} // namespace network
} // namespace btclite
