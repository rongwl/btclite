#include "msg_process_tests.h"

#include "msg_process.h"
#include "net.h"
#include "network/include/params.h"
#include "protocol/address.h"
#include "protocol/getaddr.h"
#include "protocol/inventory.h"
#include "protocol/ping.h"
#include "protocol/pong.h"
#include "protocol/reject.h"
#include "protocol/send_headers.h"
#include "protocol/send_compact.h"
#include "protocol/verack.h"
#include "protocol/version.h"
#include "random.h"


namespace btclite {
namespace unit_test {

using namespace std::placeholders;

using namespace network;
using namespace network::protocol;

uint64_t version_nonce = 0;
uint64_t ping_nonce = 0;
uint64_t pong_nonce = 0;
util::Uint256 inv_hash;
util::Uint256 reject_data;

void TestMsg(struct bufferevent *bev, void *ctx,
             std::function<void(const MessageData*)> TestMsgData)
{
    const char *msg = reinterpret_cast<const char*>(ctx);
    struct event_base *base = bufferevent_get_base(bev);
    
    ASSERT_NE(bev, nullptr);
    
    struct evbuffer *buf = bufferevent_get_input(bev);
    ASSERT_NE(buf, nullptr);
    
    uint8_t *raw = evbuffer_pullup(buf, MessageHeader::kSize);
    ASSERT_NE(raw, nullptr);
    
    MessageHeader header(raw);
    ASSERT_TRUE(header.IsValid());
    ASSERT_EQ(header.command(), msg);

    evbuffer_drain(buf, MessageHeader::kSize);
    raw = evbuffer_pullup(buf, header.payload_length());
    MessageData *message = MsgDataFactory(raw, header, kProtocolVersion);
    ASSERT_NE(message, nullptr);
    EXPECT_EQ(message->Command(), msg);
    TestMsgData(message);
    delete message;
    
    event_base_loopexit(base, NULL);
}

void TestVersion(const MessageData *msg)
{
    const Version *version = reinterpret_cast<const Version*>(msg);
    NetAddr addr_recv, addr_from;
    addr_recv.SetIpv4(inet_addr("1.2.3.4"));
    EXPECT_EQ(version->protocol_version(), kProtocolVersion);
    EXPECT_EQ(version->services(), kNodeNetwork);
    EXPECT_EQ(version->addr_recv(), addr_recv);
    EXPECT_EQ(version->addr_from(), addr_from);
    EXPECT_EQ(version->nonce(), version_nonce);
    EXPECT_EQ(version->user_agent(), protocol::FormatUserAgent());
    EXPECT_EQ(version->start_height(), 
              chain::SingletonBlockChain::GetInstance().Height());
    EXPECT_TRUE(version->relay());
}

void VersionReadCb(struct bufferevent *bev, void *ctx)
{
    TestMsg(bev, ctx, std::bind(TestVersion, _1));    
}

void TestVerack(const MessageData *msg)
{
}

void VerackReadCb(struct bufferevent *bev, void *ctx)
{
    TestMsg(bev, ctx, std::bind(TestVerack, _1));
}

void TestAddr(const MessageData *msg)
{
    const Addr *addr_msg = reinterpret_cast<const Addr*>(msg);
    NetAddr addr1, addr2;
    addr1.SetIpv4(inet_addr("1.2.3.1"));
    addr2.SetIpv4(inet_addr("1.2.3.2"));
    EXPECT_EQ(addr_msg->addr_list()[0], addr1);
    EXPECT_EQ(addr_msg->addr_list()[1], addr2);
}

void AddrReadCb(struct bufferevent *bev, void *ctx)
{
    TestMsg(bev, ctx, std::bind(TestAddr, _1));
}

void TestInv(const MessageData *msg)
{
    const Inv *inv = reinterpret_cast<const Inv*>(msg);
    EXPECT_EQ(inv->inv_vects()[0].type(), DataMsgType::kMsgTx);
    EXPECT_EQ(inv->inv_vects()[0].hash(), inv_hash);
    EXPECT_EQ(inv->inv_vects()[1].type(), DataMsgType::kMsgBlock);
    EXPECT_EQ(inv->inv_vects()[1].hash(), inv_hash);
}

void InvReadCb(struct bufferevent *bev, void *ctx)
{
    TestMsg(bev, ctx, std::bind(TestInv, _1));
}

void TestGetAddr(const MessageData *msg)
{
}

void GetAddrReadCb(struct bufferevent *bev, void *ctx)
{
    TestMsg(bev, ctx, std::bind(TestGetAddr, _1));
}

void TestPing(const MessageData *msg)
{
    const Ping *ping = reinterpret_cast<const Ping*>(msg);
    EXPECT_EQ(ping->nonce(), ping_nonce);
}

void PingReadCb(struct bufferevent *bev, void *ctx)
{
    TestMsg(bev, ctx, std::bind(TestPing, _1));
}

void TestPong(const MessageData *msg)
{
    const Pong *pong = reinterpret_cast<const Pong*>(msg);
    EXPECT_EQ(pong->nonce(), pong_nonce);
}

void PongReadCb(struct bufferevent *bev, void *ctx)
{
    TestMsg(bev, ctx, std::bind(TestPong, _1));
}

void TestRejects(const MessageData *msg)
{
    const Reject *reject1 = reinterpret_cast<const Reject*>(msg);
    Reject reject2(msg_command::kMsgBlock, CCode::kRejectInvalid, "invalid", reject_data);
    EXPECT_EQ(*reject1, reject2);
}

void RejectsReadCb(struct bufferevent *bev, void *ctx)
{
    TestMsg(bev, ctx, std::bind(TestRejects, _1));
}

void TestSendHeaders(const MessageData *msg)
{
}

void SendHeadersReadCb(struct bufferevent *bev, void *ctx)
{
    TestMsg(bev, ctx, std::bind(TestSendHeaders, _1));
}

void TestSendCmpct(const MessageData *msg)
{
    const SendCmpct *send_compact1 = reinterpret_cast<const SendCmpct*>(msg);
    SendCmpct send_compact2(true, 1);
    EXPECT_EQ(*send_compact1, send_compact2);
}

void SendCmpctReadCb(struct bufferevent *bev, void *ctx)
{
    TestMsg(bev, ctx, std::bind(TestSendCmpct, _1));
}

TEST(MsgFactoryTest, VersionFactory)
{
    util::MemOstream os;
    NetAddr addr_recv(0x1234, kNodeNetwork, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x4},
    8333);
    NetAddr addr_from(0x5678, kNodeNetwork, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x5},
    8333);
    Version msg_out(kProtocolVersion, kNodeNetwork, 0x1234, 
                    std::move(addr_recv), std::move(addr_from), 0x5678, 
                    std::move(std::string("/btclite:0.1.0/")), 1000, true);
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         msg_command::kMsgVersion, msg_out.SerializedSize(), msg_out.GetHash().GetLow32());
    
    os << msg_out;    
    MessageData *msg_in= MsgDataFactory(os.vec().data(), header, 0);
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_out, *reinterpret_cast<Version*>(msg_in));
    delete msg_in;
}

TEST(MsgFactoryTest, VerackFactory)
{
    Verack verack;
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         msg_command::kMsgVerack, 0, verack.GetHash().GetLow32());
    
    MessageData *msg_in= MsgDataFactory(nullptr, header, 0);
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_in->Command(), msg_command::kMsgVerack);
    delete msg_in;
}

TEST(MsgFactoryTest, AddrFactory)
{
    util::MemOstream os;
    Addr msg_addr;
    NetAddr addr1, addr2;
    
    addr1.SetIpv4(inet_addr("1.2.3.1"));
    addr2.SetIpv4(inet_addr("1.2.3.2"));
    msg_addr.mutable_addr_list()->push_back(addr1);
    msg_addr.mutable_addr_list()->push_back(addr2);
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         msg_command::kMsgAddr, msg_addr.SerializedSize(), msg_addr.GetHash().GetLow32());
    os << msg_addr;
    
    MessageData *msg_in= MsgDataFactory(os.vec().data(), header, 0);
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(*reinterpret_cast<Addr*>(msg_in), msg_addr);
    delete msg_in;
}

TEST(MsgFactoryTest, GetAddrFactory)
{
    GetAddr getaddr;
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         msg_command::kMsgGetAddr, 0, getaddr.GetHash().GetLow32());
    
    MessageData *msg_in= MsgDataFactory(nullptr, header, 0);
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_in->Command(), msg_command::kMsgGetAddr);
    delete msg_in;
}

TEST(MsgFactoryTest, InvFactory)
{
    util::MemOstream os;
    Inv msg_out;
    msg_out.mutable_inv_vects()->emplace_back(DataMsgType::kMsgTx, 
                                              util::GetUint256());
    msg_out.mutable_inv_vects()->emplace_back(DataMsgType::kMsgBlock, 
                                              util::GetUint256());
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         msg_command::kMsgInv, msg_out.SerializedSize(), msg_out.GetHash().GetLow32());
    
    os << msg_out;  
    MessageData *msg_in= MsgDataFactory(os.vec().data(), header, 0);
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_out, *reinterpret_cast<Inv*>(msg_in));
    delete msg_in;    
}

TEST(MsgFactoryTest, PingFactory)
{
    util::MemOstream ms;    
    Ping msg_out(0x1122334455667788);
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         msg_command::kMsgPing, msg_out.SerializedSize(), msg_out.GetHash().GetLow32());
    
    ms << msg_out;
    MessageData *msg_in = MsgDataFactory( ms.vec().data(), header, msg_out.protocol_version());
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_out, *reinterpret_cast<Ping*>(msg_in));
    delete msg_in;
    
    ms.Clear();
    msg_out.set_protocol_version(0);
    header.set_payload_length(msg_out.SerializedSize());
    header.set_checksum(msg_out.GetHash().GetLow32());
    ms << msg_out;
    msg_in = MsgDataFactory(nullptr, header, msg_out.protocol_version());
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(reinterpret_cast<Ping*>(msg_in)->nonce(), 0);
    delete msg_in;
}

TEST(MsgFactoryTest, PongFactory)
{
    util::MemOstream ms;    
    Pong msg_out(0x1122334455667788);
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         msg_command::kMsgPong, msg_out.SerializedSize(), msg_out.GetHash().GetLow32());
    
    ms << msg_out;
    MessageData *msg_in = MsgDataFactory(ms.vec().data(), header, 0);
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_out, *reinterpret_cast<Pong*>(msg_in));
    delete msg_in;
}

TEST(MsgFactoryTest, RejectFactory)
{
    util::MemOstream os;
    Reject msg_out(msg_command::kMsgVersion, CCode::kRejectDuplicate, "Duplicate version message",
                   std::move(util::GetUint256()));
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         msg_command::kMsgReject, msg_out.SerializedSize(), msg_out.GetHash().GetLow32());
    
    os << msg_out;    
    MessageData *msg_in = MsgDataFactory(os.vec().data(), header, 0);
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_out, *reinterpret_cast<Reject*>(msg_in));
    delete msg_in;
}

TEST(MsgFactoryTest, SendHeadersFactory)
{
    util::MemOstream os;
    SendHeaders send_headers;
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         msg_command::kMsgSendHeaders, send_headers.SerializedSize(), 
                         send_headers.GetHash().GetLow32());
    
    os << send_headers;    
    MessageData *msg = MsgDataFactory(os.vec().data(), header, 0);
    ASSERT_NE(msg, nullptr);
    EXPECT_EQ(msg->Command(), msg_command::kMsgSendHeaders);
    delete msg;
}

TEST(MsgFactoryTest, SendCmpctFactory)
{
    util::MemOstream os;
    SendCmpct send_compact(true, 1);
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         msg_command::kMsgSendCmpct, send_compact.SerializedSize(), 
                         send_compact.GetHash().GetLow32());
    
    os << send_compact;    
    MessageData *msg = MsgDataFactory( os.vec().data(), header, 0);
    ASSERT_NE(msg, nullptr);
    EXPECT_EQ(*reinterpret_cast<SendCmpct*>(msg), send_compact);
}

TEST(MsgFactoryTest, NullFactory)
{
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         msg_command::kMsgVersion, 0, 0);
    MessageData *msg_in = MsgDataFactory(nullptr, header, 0);
    EXPECT_EQ(msg_in, nullptr);
    
    uint8_t foo;
    header.set_command("bar");
    msg_in = MsgDataFactory(&foo, header, 0);
    EXPECT_EQ(msg_in, nullptr);
}

TEST_F(MsgProcessTest, SendVersion)
{
    ASSERT_NE(pair_[0], nullptr);
    ASSERT_NE(pair_[1], nullptr);
    ASSERT_TRUE(addr_.IsValid());
    
    bufferevent_setcb(pair_[1], VersionReadCb, NULL, NULL, const_cast<char*>(msg_command::kMsgVersion));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    node->mutable_protocol()->set_services(kNodeNetwork);
    version_nonce = node->local_host_nonce();
    ASSERT_TRUE(SendVersion(node));
    
    event_base_dispatch(base_);
    
    //bufferevent_free(pair_[0]);
    //bufferevent_free(pair_[1]);
    //event_base_free(base_);
}

TEST_F(MsgProcessTest, SendVerack)
{
    bufferevent_setcb(pair_[1], VerackReadCb, NULL, NULL, const_cast<char*>(msg_command::kMsgVerack));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    Verack verack;
    ASSERT_TRUE(SendMsg(verack, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendAddr)
{
    bufferevent_setcb(pair_[1], AddrReadCb, NULL, NULL, const_cast<char*>(msg_command::kMsgAddr));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    NetAddr addr;
    Addr addr_msg;
    
    addr.SetIpv4(inet_addr("1.2.3.1"));
    addr_msg.mutable_addr_list()->push_back(addr);
    addr.SetIpv4(inet_addr("1.2.3.2"));
    addr_msg.mutable_addr_list()->push_back(addr);
    ASSERT_TRUE(SendMsg(addr_msg, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendInv)
{
    bufferevent_setcb(pair_[1], InvReadCb, NULL, NULL, const_cast<char*>(msg_command::kMsgInv));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    inv_hash = util::GetUint256();
    Inv inv;
    inv.mutable_inv_vects()->emplace_back(DataMsgType::kMsgTx, inv_hash);
    inv.mutable_inv_vects()->emplace_back(DataMsgType::kMsgBlock, inv_hash);
    ASSERT_TRUE(SendMsg(inv, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendGetAddr)
{
    bufferevent_setcb(pair_[1], GetAddrReadCb, NULL, NULL, const_cast<char*>(msg_command::kMsgGetAddr));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    GetAddr getaddr;
    ASSERT_TRUE(SendMsg(getaddr, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendPing)
{
    bufferevent_setcb(pair_[1], PingReadCb, NULL, NULL, const_cast<char*>(msg_command::kMsgPing));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    ping_nonce = util::GetUint64();
    Ping ping(ping_nonce);
    ASSERT_TRUE(SendMsg(ping, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendPong)
{
    bufferevent_setcb(pair_[1], PongReadCb, NULL, NULL, const_cast<char*>(msg_command::kMsgPong));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    pong_nonce = util::GetUint64();
    Pong pong(pong_nonce);
    ASSERT_TRUE(SendMsg(pong, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendSendHeaders)
{
    bufferevent_setcb(pair_[1], SendHeadersReadCb, NULL, NULL, const_cast<char*>(msg_command::kMsgSendHeaders));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    SendHeaders send_headers;
    ASSERT_TRUE(SendMsg(send_headers, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendSendCmpct)
{
    bufferevent_setcb(pair_[1], SendCmpctReadCb, NULL, NULL, const_cast<char*>(msg_command::kMsgSendCmpct));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    SendCmpct send_compact(true, 1);
    ASSERT_TRUE(SendMsg(send_compact, node));
    
    event_base_dispatch(base_);
}

} // namespace unit_test
} // namespace btclit
