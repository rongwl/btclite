#include "msg_process_tests.h"

#include "msg_process.h"
#include "net.h"
#include "network/include/params.h"
#include "protocol/address.h"
#include "protocol/getaddr.h"
#include "protocol/inventory.h"
#include "protocol/ping.h"
#include "protocol/reject.h"
#include "protocol/send_headers.h"
#include "protocol/send_compact.h"
#include "protocol/verack.h"
#include "protocol/version.h"
#include "random.h"


using namespace std::placeholders;

using namespace btclite::network;
using namespace btclite::network::msg_process;
using namespace btclite::network::protocol;

uint64_t version_nonce = 0;
uint64_t ping_nonce = 0;
Uint256 inv_hash;
Uint256 reject_data;

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
    MessageData *message = MsgDataFactory(header, raw);
    ASSERT_NE(message, nullptr);
    EXPECT_EQ(message->Command(), msg);
    TestMsgData(message);
    delete message;
    
    event_base_loopexit(base, NULL);
}

void TestVersion(const MessageData *msg)
{
    const Version *version = reinterpret_cast<const Version*>(msg);
    btclite::network::NetAddr addr_recv, addr_from;
    addr_recv.SetIpv4(inet_addr("1.2.3.4"));
    EXPECT_EQ(version->protocol_version(), kProtocolVersion);
    EXPECT_EQ(version->services(),
              SingletonLocalNetCfg::GetInstance().local_services());
    EXPECT_EQ(version->addr_recv(), addr_recv);
    EXPECT_EQ(version->addr_from(), addr_from);
    EXPECT_EQ(version->nonce(), version_nonce);
    EXPECT_EQ(version->user_agent(), FormatUserAgent());
    EXPECT_EQ(version->start_height(), 
              SingletonBlockChain::GetInstance().Height());
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
    btclite::network::NetAddr addr1, addr2;
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

void TestRejects(const MessageData *msg)
{
    const Reject *reject1 = reinterpret_cast<const Reject*>(msg);
    Reject reject2(::kMsgBlock, kRejectInvalid, "invalid", reject_data);
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
                         kMsgVersion, msg_out.SerializedSize(), msg_out.GetHash().GetLow32());
    
    os << msg_out;    
    MessageData *msg_in= MsgDataFactory(header, os.vec().data());
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_out, *reinterpret_cast<Version*>(msg_in));
    delete msg_in;
}

TEST(MsgFactoryTest, VerackFactory)
{
    Verack verack;
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         kMsgVerack, 0, verack.GetHash().GetLow32());
    
    MessageData *msg_in= MsgDataFactory(header, nullptr);
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_in->Command(), kMsgVerack);
    delete msg_in;
}

TEST(MsgFactoryTest, AddrFactory)
{
    MemOstream os;
    Addr msg_addr;
    btclite::network::NetAddr addr1, addr2;
    
    addr1.SetIpv4(inet_addr("1.2.3.1"));
    addr2.SetIpv4(inet_addr("1.2.3.2"));
    msg_addr.mutable_addr_list()->push_back(addr1);
    msg_addr.mutable_addr_list()->push_back(addr2);
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         kMsgAddr, msg_addr.SerializedSize(), msg_addr.GetHash().GetLow32());
    os << msg_addr;
    
    MessageData *msg_in= MsgDataFactory(header, os.vec().data());
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(*reinterpret_cast<Addr*>(msg_in), msg_addr);
    delete msg_in;
}

TEST(MsgFactoryTest, GetAddrFactory)
{
    GetAddr getaddr;
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         kMsgGetAddr, 0, getaddr.GetHash().GetLow32());
    
    MessageData *msg_in= MsgDataFactory(header, nullptr);
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_in->Command(), kMsgGetAddr);
    delete msg_in;
}

TEST(MsgFactoryTest, InvFactory)
{
    MemOstream os;
    Inv msg_out;
    msg_out.mutable_inv_vects()->emplace_back(DataMsgType::kMsgTx, 
                                              btclite::utility::random::GetUint256());
    msg_out.mutable_inv_vects()->emplace_back(DataMsgType::kMsgBlock, 
                                              btclite::utility::random::GetUint256());
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         kMsgInv, msg_out.SerializedSize(), msg_out.GetHash().GetLow32());
    
    os << msg_out;  
    MessageData *msg_in= MsgDataFactory(header, os.vec().data());
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_out, *reinterpret_cast<Inv*>(msg_in));
    delete msg_in;    
}

TEST(MsgFactoryTest, PingFactory)
{
    MemOstream os;
    Ping msg_out(0x1122334455667788);
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         kMsgPing, msg_out.SerializedSize(), msg_out.GetHash().GetLow32());
    
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
                         kMsgReject, msg_out.SerializedSize(), msg_out.GetHash().GetLow32());
    
    os << msg_out;    
    MessageData *msg_in= MsgDataFactory(header, os.vec().data());
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_out, *reinterpret_cast<Reject*>(msg_in));
    delete msg_in;
}

TEST(MsgFactoryTest, SendHeadersFactory)
{
    MemOstream os;
    SendHeaders send_headers;
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         kMsgSendHeaders, send_headers.SerializedSize(), 
                         send_headers.GetHash().GetLow32());
    
    os << send_headers;    
    MessageData *msg= MsgDataFactory(header, os.vec().data());
    ASSERT_NE(msg, nullptr);
    EXPECT_EQ(msg->Command(), kMsgSendHeaders);
    delete msg;
}

TEST(MsgFactoryTest, SendCmpctFactory)
{
    MemOstream os;
    SendCmpct send_compact(true, 1);
    MessageHeader header(SingletonParams::GetInstance().msg_magic(),
                         kMsgSendCmpct, send_compact.SerializedSize(), 
                         send_compact.GetHash().GetLow32());
    
    os << send_compact;    
    MessageData *msg= MsgDataFactory(header, os.vec().data());
    ASSERT_NE(msg, nullptr);
    EXPECT_EQ(*reinterpret_cast<SendCmpct*>(msg), send_compact);
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
    
    bufferevent_setcb(pair_[1], VersionReadCb, NULL, NULL, const_cast<char*>(kMsgVersion));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    version_nonce = node->local_host_nonce();
    ASSERT_TRUE(msg_process::SendVersion(node));
    
    event_base_dispatch(base_);
    
    //bufferevent_free(pair_[0]);
    //bufferevent_free(pair_[1]);
    //event_base_free(base_);
}

TEST_F(MsgProcessTest, SendVerack)
{
    bufferevent_setcb(pair_[1], VerackReadCb, NULL, NULL, const_cast<char*>(kMsgVerack));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    Verack verack;
    ASSERT_TRUE(msg_process::SendMsg(verack, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendAddr)
{
    bufferevent_setcb(pair_[1], AddrReadCb, NULL, NULL, const_cast<char*>(kMsgAddr));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    btclite::network::NetAddr addr;
    Addr addr_msg;
    
    addr.SetIpv4(inet_addr("1.2.3.1"));
    addr_msg.mutable_addr_list()->push_back(addr);
    addr.SetIpv4(inet_addr("1.2.3.2"));
    addr_msg.mutable_addr_list()->push_back(addr);
    ASSERT_TRUE(msg_process::SendMsg(addr_msg, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendInv)
{
    bufferevent_setcb(pair_[1], InvReadCb, NULL, NULL, const_cast<char*>(kMsgInv));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    inv_hash = btclite::utility::random::GetUint256();
    Inv inv;
    inv.mutable_inv_vects()->emplace_back(DataMsgType::kMsgTx, inv_hash);
    inv.mutable_inv_vects()->emplace_back(DataMsgType::kMsgBlock, inv_hash);
    ASSERT_TRUE(msg_process::SendMsg(inv, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendGetAddr)
{
    bufferevent_setcb(pair_[1], GetAddrReadCb, NULL, NULL, const_cast<char*>(kMsgGetAddr));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    GetAddr getaddr;
    ASSERT_TRUE(msg_process::SendMsg(getaddr, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendPing)
{
    bufferevent_setcb(pair_[1], PingReadCb, NULL, NULL, const_cast<char*>(kMsgPing));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    ping_nonce = btclite::utility::random::GetUint64();
    Ping ping(ping_nonce);
    ASSERT_TRUE(msg_process::SendMsg(ping, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendSendHeaders)
{
    bufferevent_setcb(pair_[1], SendHeadersReadCb, NULL, NULL, const_cast<char*>(kMsgSendHeaders));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    SendHeaders send_headers;
    ASSERT_TRUE(msg_process::SendMsg(send_headers, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendSendCmpct)
{
    bufferevent_setcb(pair_[1], SendCmpctReadCb, NULL, NULL, const_cast<char*>(kMsgSendCmpct));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    SendCmpct send_compact(true, 1);
    ASSERT_TRUE(msg_process::SendMsg(send_compact, node));
    
    event_base_dispatch(base_);
}
