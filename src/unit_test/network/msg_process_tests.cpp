#include "msg_process_tests.h"

#include "msg_process.h"
#include "network/include/params.h"
#include "protocol/ping.h"
#include "protocol/reject.h"
#include "protocol/version.h"
#include "random.h"


using namespace btclite::network;
using namespace btclite::network::msgprocess;
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

TEST_F(FixtureMsgProcessTest, SendVersion)
{
    ASSERT_NE(pair_[0], nullptr);
    ASSERT_NE(pair_[1], nullptr);
    ASSERT_TRUE(addr_.IsValid());
    
    bufferevent_setcb(pair_[1], ReadCb, NULL, NULL, const_cast<char*>(kMsgVersion));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    ASSERT_TRUE(msgprocess::SendVersion(node));
    
    event_base_dispatch(base_);
    
    //bufferevent_free(pair[0]);
    //bufferevent_free(pair[1]);
    //event_base_free(base);
}

TEST_F(FixtureMsgProcessTest, SendRejects)
{
    ASSERT_NE(pair_[0], nullptr);
    ASSERT_NE(pair_[1], nullptr);
    ASSERT_TRUE(addr_.IsValid());
    
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    SingletonNodes::GetInstance().AddNode(node);
    SingletonBlockSync::GetInstance().AddSyncState(node->id(), addr_, "");
    BlockSyncState *state = SingletonBlockSync::GetInstance().GetSyncState(node->id());
    ASSERT_NE(state, nullptr);
    
    BlockReject block_reject = { kRejectInvalid, "invalid", btclite::utility::random::GetUint256() };
    state->mutable_stats()->AddBlockReject(std::move(block_reject));    
    bufferevent_setcb(pair_[1], ReadCb, NULL, NULL, const_cast<char*>(kMsgReject));
    bufferevent_enable(pair_[1], EV_READ);
    ASSERT_TRUE(msgprocess::SendRejects(node));
    
    event_base_dispatch(base_);
}
