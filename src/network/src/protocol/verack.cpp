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

namespace private_verack {

bool Verack::RecvHandler(std::shared_ptr<Node> src_node) const
{
    if (src_node->protocol().version() >= kSendheadersVersion) {
        // Tell our peer we prefer to receive headers rather than inv's
        // We send this to non-NODE NETWORK peers as well, because even
        // non-NODE NETWORK peers can announce blocks (such as pruning
        // nodes)
        SendHeaders send_headers;
        SendMsg(send_headers, src_node);
    }
    if (src_node->protocol().version() >= kShortIdsBlocksVersion) {
        // Tell our peer we are willing to provide version 1 or 2 cmpctblocks
        // However, we do not request new block announcements using
        // cmpctblock messages.
        // We send this to non-NODE NETWORK peers as well, because
        // they may wish to request compact blocks from us
        SendCmpct send_compact(false, 2);
        if (SingletonLocalNetCfg::GetInstance().local_services() & ServiceFlags::kNodeWitness)
            SendMsg(send_compact, src_node);
        send_compact.set_version(1);
        SendMsg(send_compact, src_node);
    }
    src_node->mutable_connection()->set_established(true);
    
    // send ping
    Ping ping(util::RandUint64());
    if (src_node->protocol().version() < kBip31Version)
        ping.set_protocol_version(0);
    SendMsg(ping, src_node);
    // start ping timer
    src_node->mutable_timers()->ping_timer = 
        util::SingletonTimerMng::GetInstance().StartTimer(kNoPingTimeout*1000, kNoPingTimeout*1000,
                                                    Ping::PingTimeoutCb, src_node);
    
    // broadcast local address
    AdvertiseLocalAddr(src_node);
    src_node->mutable_timers()->broadcast_local_addr_timer =
        util::SingletonTimerMng::GetInstance().StartTimer(IntervalNextSend(kAvgLocalAddrBcInterval)*1000, 0, 
                                                    LocalNetConfig::BroadcastTimeoutCb,
                                                    src_node);
    
    // broadcast address
    src_node->mutable_timers()->broadcast_addr_timer = 
        util::SingletonTimerMng::GetInstance().StartTimer(kAvgAddrBcInterval*1000, 0, 
                                                    BroadcastAddrsTimeoutCb, src_node);
    
    return true;
}

} // namespace private_verack

} // namespace protocol
} // namespace network
} // namespace btclite
