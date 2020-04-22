#ifndef BTCLITE_MSG_PROCESS_H
#define BTCLITE_MSG_PROCESS_H

#include "hash.h"
#include "network/include/params.h"
#include "protocol/message.h"
#include "stream.h"


namespace btclite {
namespace network {

protocol::MessageData *MsgDataFactory(const uint8_t *raw, 
                                      const protocol::MessageHeader& header,
                                      uint32_t protocol_version);
bool ParseMsg(std::shared_ptr<Node> src_node, const Params& params);

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
                    kNoSendingTimeout*1000, 0, NodeTimeoutCb::InactivityTimeoutCb, dst_node);
    }

    return true;
}

bool SendVersion(std::shared_ptr<Node> dst_node, uint32_t magic);
bool SendAddr(std::shared_ptr<Node> dst_node);


} // namespace network
} // namespace btclite

#endif // BTCLITE_MSG_PROCESS_H
