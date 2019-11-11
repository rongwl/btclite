#include "msg_process_tests.h"

#include "msg_process.h"
#include "network/include/params.h"
#include "protocol/ping.h"
#include "protocol/reject.h"
#include "protocol/verack.h"
#include "protocol/version.h"
#include "random.h"


using namespace btclite::network;
using namespace btclite::network::msg_process;
using namespace btclite::network::protocol;

void CheckMsg(struct bufferevent *bev, const char msg[])
{    
    ASSERT_NE(bev, nullptr);
    
    struct evbuffer *buf = bufferevent_get_input(bev);
    ASSERT_NE(buf, nullptr);
    
    uint8_t *raw = evbuffer_pullup(buf, MessageHeader::kSize);
    ASSERT_NE(raw, nullptr);
    
    MessageHeader header;
    ASSERT_TRUE(header.Init(raw));
    ASSERT_EQ(header.command(), msg);

    evbuffer_drain(buf, MessageHeader::kSize);
    raw = evbuffer_pullup(buf, header.payload_length());
    MessageData *message = MsgDataFactory(header, raw);
    ASSERT_NE(message, nullptr);
    EXPECT_EQ(message->Command(), msg);
    delete message;
}

void ReadCb(struct bufferevent *bev, void *ctx)
{
    const char *msg = reinterpret_cast<const char*>(ctx);
    struct event_base *base = bufferevent_get_base(bev);
    NetAddr addr;
    
    addr.SetIpv4(inet_addr("1.2.3.5"));
    auto node = std::make_shared<Node>(bev, addr, false);
    CheckMsg(bev, msg);
    
    event_base_loopexit(base, NULL);
}

TEST(MsgFactoryTest, VersionFactory)
{
    MemOstream os;
    btclite::network::NetAddr addr_recv(0x1234, kNodeNetwork, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x4},
    8333);
    btclite::network::NetAddr addr_from(0x5678, kNodeNetwork, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x5},
    8333);
    Version msg_out(kProtocolVersion, kNodeNetwork, 0x1234, std::move(addr_recv),
                    std::move(addr_from), 0x5678, std::move(std::string("/btclite:0.1.0/")),
                    1000, true);
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         kMsgVersion, msg_out.SerializedSize(), 0);
    
    os << msg_out;    
    MessageData *msg_in= MsgDataFactory(header, os.vec().data());
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_out, *reinterpret_cast<Version*>(msg_in));
    delete msg_in;
}

TEST(MsgFactoryTest, VerackFactory)
{
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         kMsgVerack, 0, 0);
    
    MessageData *msg_in= MsgDataFactory(header, nullptr);
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_in->Command(), kMsgVerack);
    delete msg_in;
}

TEST(MsgFactoryTest, PingFactory)
{
    MemOstream os;
    Ping msg_out(0x1122334455667788);
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         kMsgPing, msg_out.SerializedSize(), 0);
    
    os << msg_out;    
    MessageData *msg_in= MsgDataFactory(header, os.vec().data());
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_out, *reinterpret_cast<Ping*>(msg_in));
    delete msg_in;
}

TEST(MsgFactoryTest, RejectFactory)
{
    MemOstream os;
    Reject msg_out(kMsgVersion, kRejectDuplicate, "Duplicate version message",
                   std::move(btclite::utility::random::GetUint256()));
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         kMsgReject, msg_out.SerializedSize(), 0);
    
    os << msg_out;    
    MessageData *msg_in= MsgDataFactory(header, os.vec().data());
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_out, *reinterpret_cast<Reject*>(msg_in));
    delete msg_in;
}

TEST(MsgFactoryTest, NullFactory)
{
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         kMsgVersion, 0, 0);
    MessageData *msg_in= MsgDataFactory(header, nullptr);
    EXPECT_EQ(msg_in, nullptr);
    
    uint8_t foo;
    header.set_command("bar");
    msg_in= MsgDataFactory(header, &foo);
    EXPECT_EQ(msg_in, nullptr);
}

TEST_F(MsgProcessTest, SendVersion)
{
    ASSERT_NE(pair_[0], nullptr);
    ASSERT_NE(pair_[1], nullptr);
    ASSERT_TRUE(addr_.IsValid());
    
    bufferevent_setcb(pair_[1], ReadCb, NULL, NULL, const_cast<char*>(kMsgVersion));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    ASSERT_TRUE(msg_process::SendVersion(node));
    
    event_base_dispatch(base_);
    
    //bufferevent_free(pair_[0]);
    //bufferevent_free(pair_[1]);
    //event_base_free(base_);
}

TEST_F(MsgProcessTest, SendVerack)
{
    ASSERT_NE(pair_[0], nullptr);
    ASSERT_NE(pair_[1], nullptr);
    ASSERT_TRUE(addr_.IsValid());
    
    bufferevent_setcb(pair_[1], ReadCb, NULL, NULL, const_cast<char*>(kMsgVerack));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    Verack verack;
    ASSERT_TRUE(msg_process::SendMsg(verack, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendRejects)
{
    ASSERT_NE(pair_[0], nullptr);
    ASSERT_NE(pair_[1], nullptr);
    ASSERT_TRUE(addr_.IsValid());
    
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    SingletonNodes::GetInstance().AddNode(node);
    SingletonBlockSync::GetInstance().AddSyncState(node->id(), addr_, "");    
    BlockReject block_reject = { kRejectInvalid, "invalid", btclite::utility::random::GetUint256() };
    ASSERT_TRUE(SingletonBlockSync::GetInstance().AddBlockReject(node->id(), block_reject));
    
    bufferevent_setcb(pair_[1], ReadCb, NULL, NULL, const_cast<char*>(kMsgReject));
    bufferevent_enable(pair_[1], EV_READ);
    ASSERT_TRUE(msg_process::SendRejects(node));
    
    event_base_dispatch(base_);
    SingletonBlockSync::GetInstance().Clear();
    SingletonNodes::GetInstance().Clear();
}
