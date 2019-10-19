#include <gtest/gtest.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "msg_process.h"
#include "network/include/params.h"
#include "protocol/ping.h"
#include "protocol/reject.h"
#include "protocol/version.h"
#include "random.h"


using namespace btclite::network;
using namespace btclite::network::msgprocess;
using namespace btclite::network::protocol;

void ReadCb(struct bufferevent *bev, void *ctx)
{
    struct event_base *base = reinterpret_cast<struct event_base*>(ctx);
    NetAddr addr;
    
    addr.SetIpv4(inet_addr("1.2.3.5"));
    auto node = std::make_shared<Node>(bev, addr, false);
    EXPECT_TRUE(msgprocess::ParseMsg(node));
    
    event_base_loopexit(base, NULL);
}

TEST(MsgProcessTest, VersionFactory)
{
    MemOstream os;
    btclite::network::NetAddr addr_recv(0x1234, kNodeNetwork, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x4},
    8333);
    btclite::network::NetAddr addr_from(0x5678, kNodeNetwork, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x5},
    8333);
    Version msg_out;
    msg_out.set_version(kProtocolVersion);
    msg_out.set_services(kNodeNetwork);
    msg_out.set_timestamp(0x1234);
    msg_out.set_addr_recv(std::move(addr_recv));
    msg_out.set_addr_from(std::move(addr_from));
    msg_out.set_nonce(0x5678);
    msg_out.set_user_agent(std::move(std::string("/btclite:0.1.0/")));
    msg_out.set_start_height(1000);
    msg_out.set_relay(true);
    os << msg_out;
    
    MessageHeader header;
    header.set_magic(SingletonParams::GetInstance().msg_magic());
    header.set_command(kMsgVersion);
    header.set_payload_length(msg_out.SerializedSize());
    MessageData *msg_in= MsgDataFactory(header, os.vec().data());
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_out, *reinterpret_cast<Version*>(msg_in));
    delete msg_in;
}

TEST(MsgProcessTest, PingFactory)
{
    MemOstream os;
    Ping msg_out;
    
    msg_out.set_nonce(0x1122334455667788);
    os << msg_out;
    
    MessageHeader header;
    header.set_magic(SingletonParams::GetInstance().msg_magic());
    header.set_command(kMsgPing);
    header.set_payload_length(msg_out.SerializedSize());
    MessageData *msg_in= MsgDataFactory(header, os.vec().data());
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_out, *reinterpret_cast<Ping*>(msg_in));
    delete msg_in;
}

TEST(MsgProcessTest, RejectFactory)
{
    MemOstream os;
    Reject msg_out;
    
    msg_out.set_message(kMsgVersion);
    msg_out.set_ccode(kRejectDuplicate);
    msg_out.set_reason("Duplicate version message");
    msg_out.set_data(std::move(btclite::utility::random::GetUint256()));
    os << msg_out;
    
    MessageHeader header;
    header.set_magic(SingletonParams::GetInstance().msg_magic());
    header.set_command(kMsgReject);
    header.set_payload_length(msg_out.SerializedSize());
    MessageData *msg_in= MsgDataFactory(header, os.vec().data());
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_out, *reinterpret_cast<Reject*>(msg_in));
    delete msg_in;
}

TEST(MsgProcessTest, NullFactory)
{
    MessageHeader header;
    header.set_magic(SingletonParams::GetInstance().msg_magic());
    header.set_command(kMsgVersion);
    MessageData *msg_in= MsgDataFactory(header, nullptr);
    EXPECT_EQ(msg_in, nullptr);
    
    uint8_t foo;
    header.set_command("bar");
    msg_in= MsgDataFactory(header, &foo);
    EXPECT_EQ(msg_in, nullptr);
}

TEST(MsgProcessTest, SendAndParseMsg)
{
    struct event_base *base;
    struct bufferevent *pair[2];
    NetAddr addr;
    
    base = event_base_new();
    ASSERT_NE(base, nullptr);
    ASSERT_EQ(bufferevent_pair_new(base, BEV_OPT_CLOSE_ON_FREE, pair), 0);
    
    addr.SetIpv4(inet_addr("1.2.3.4"));
    auto node = std::make_shared<Node>(pair[0], addr, false);
    ASSERT_TRUE(msgprocess::SendVerMsg(node));
    
    bufferevent_setcb(pair[1], ReadCb, NULL, NULL, base);
    bufferevent_enable(pair[1], EV_READ);
    event_base_dispatch(base);
    
    //bufferevent_free(pair[0]);
    //bufferevent_free(pair[1]);
    //event_base_free(base);
}
