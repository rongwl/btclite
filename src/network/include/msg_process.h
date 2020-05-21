#ifndef BTCLITE_MSG_PROCESS_H
#define BTCLITE_MSG_PROCESS_H

#include "hash.h"
#include "net.h"
#include "network/include/params.h"
#include "protocol/message.h"
#include "stream.h"


namespace btclite {
namespace network {

bool ParseMsg(std::shared_ptr<Node> src_node, const Params& params, 
              const LocalService& local_service, Peers *ppeers);

template <typename Message>
bool HandleMsgData(std::shared_ptr<Node> src_node, 
                   const protocol::MessageHeader& header, const Message& msg, 
                   std::function<bool(std::shared_ptr<Node>)> recv_handler)
{
    if (header.checksum() != msg.GetHash().GetLow32()) {
        BTCLOG(LOG_LEVEL_WARNING) << msg.Command() << " message checksum error: expect "
                                  << header.checksum() << ", received "
                                  << msg.GetHash().GetLow32();
        return false;
    }
    
    if (!msg.IsValid()) {
        BTCLOG(LOG_LEVEL_ERROR) << "Received invalid " << msg.Command() 
                                << " message data";
        return false;
    }
    
    if (!protocol::CheckMisbehaving(msg.Command(), src_node)) {
        BTCLOG(LOG_LEVEL_ERROR) << "Received misbehavior message from peer "
                                << src_node->id();
        src_node->mutable_misbehavior()->Misbehaving(src_node->id(), 1);
        return false;
    }
    
    return recv_handler(src_node);
}

template <typename Message>
bool SendMsg(const Message& msg, uint32_t magic, std::shared_ptr<Node> dst_node)
{
    using namespace protocol;
    
    util::MemoryStream ms;
        
    if (!dst_node->connection().bev())
        return false;
    
    if (dst_node->connection().socket_no_msg()) {
        dst_node->mutable_connection()->set_socket_no_msg(false);
    }

    MessageHeader header(magic, msg.Command(), msg.SerializedSize(), msg.GetHash().GetLow32());
    ms << header << msg;

    if (ms.Size() != MessageHeader::kSize + msg.SerializedSize()) {
        BTCLOG(LOG_LEVEL_ERROR) << "Wrong serialized message size:" << ms.Size()
                                << ", correct size:" << MessageHeader::kSize + msg.SerializedSize() 
                                << ", message type:" << msg.Command();
        return false;
    }

    //bufferevent_lock(dst_node->mutable_bev());

    if (bufferevent_write(dst_node->mutable_connection()->mutable_bev(), ms.Data(), ms.Size())) {
        BTCLOG(LOG_LEVEL_ERROR) << "Writing message to bufferevent failed, peer:" 
                                << dst_node->id();
        return false;
    }

    //bufferevent_unlock(dst_node->mutable_bev());
    
    if (dst_node->timers().no_sending_timer) {
        dst_node->mutable_timers()->no_sending_timer->Reset();
    }
    else {
        util::TimerMng& timer_mng = util::SingletonTimerMng::GetInstance();
        dst_node->mutable_timers()->no_sending_timer = timer_mng.StartTimer(
                    kNoSendingTimeout*1000, 0, std::bind(&Node::InactivityTimeoutCb, dst_node));
    }

    return true;
}

bool SendVersion(std::shared_ptr<Node> dst_node, uint32_t magic);
bool SendAddr(std::shared_ptr<Node> dst_node);


} // namespace network
} // namespace btclite

#endif // BTCLITE_MSG_PROCESS_H
