#include "msg_process_tests.h"

#include "chain_state.h"
#include "msg_process.h"
#include "net.h"
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

#if 0
TEST(MsgFactoryTest, VersionFactory)
{
    util::MemoryStream ms;
    NetAddr addr_recv(0x1234, kNodeNetwork, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x4},
    8333);
    NetAddr addr_from(0x5678, kNodeNetwork, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x5},
    8333);
    Version msg_out(kProtocolVersion, kNodeNetwork, 0x1234, 
                    std::move(addr_recv), std::move(addr_from), 0x5678, 
                    std::move(std::string("/btclite:0.1.0/")), 1000, true);
    MessageHeader header(kMainnetMagic, msg_command::kMsgVersion, 
                         msg_out.SerializedSize(), msg_out.GetHash().GetLow32());
    
    ms << msg_out;    
    MessageData *msg_in= MsgDataFactory(ms.Data(), header, 0);
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_out, *reinterpret_cast<Version*>(msg_in));
    delete msg_in;
}

TEST(MsgFactoryTest, VerackFactory)
{
    Verack verack;
    MessageHeader header(kMainnetMagic, msg_command::kMsgVerack, 0, 
                         verack.GetHash().GetLow32());
    
    MessageData *msg_in= MsgDataFactory(nullptr, header, 0);
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_in->Command(), msg_command::kMsgVerack);
    delete msg_in;
}

TEST(MsgFactoryTest, AddrFactory)
{
    util::MemoryStream ms;
    Addr msg_addr;
    NetAddr addr1, addr2;
    
    addr1.SetIpv4(inet_addr("1.2.3.1"));
    addr2.SetIpv4(inet_addr("1.2.3.2"));
    msg_addr.mutable_addr_list()->push_back(addr1);
    msg_addr.mutable_addr_list()->push_back(addr2);
    MessageHeader header(kMainnetMagic, msg_command::kMsgAddr, msg_addr.SerializedSize(), 
                         msg_addr.GetHash().GetLow32());
    ms << msg_addr;
    
    MessageData *msg_in= MsgDataFactory(ms.Data(), header, 0);
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(*reinterpret_cast<Addr*>(msg_in), msg_addr);
    delete msg_in;
}

TEST(MsgFactoryTest, GetAddrFactory)
{
    GetAddr getaddr;
    MessageHeader header(kMainnetMagic, msg_command::kMsgGetAddr, 0, 
                         getaddr.GetHash().GetLow32());
    
    MessageData *msg_in= MsgDataFactory(nullptr, header, 0);
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_in->Command(), msg_command::kMsgGetAddr);
    delete msg_in;
}

TEST(MsgFactoryTest, InvFactory)
{
    util::MemoryStream ms;
    Inv msg_out;
    msg_out.mutable_inv_vects()->emplace_back(DataMsgType::kMsgTx, 
                                              util::RandHash256());
    msg_out.mutable_inv_vects()->emplace_back(DataMsgType::kMsgBlock, 
                                              util::RandHash256());
    MessageHeader header(kMainnetMagic, msg_command::kMsgInv, msg_out.SerializedSize(), 
                         msg_out.GetHash().GetLow32());
    
    ms << msg_out;  
    MessageData *msg_in= MsgDataFactory(ms.Data(), header, 0);
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_out, *reinterpret_cast<Inv*>(msg_in));
    delete msg_in;    
}

TEST(MsgFactoryTest, PingFactory)
{
    util::MemoryStream ms;    
    Ping msg_out(0x1122334455667788);
    MessageHeader header(kMainnetMagic, msg_command::kMsgPing, msg_out.SerializedSize(),
                         msg_out.GetHash().GetLow32());
    
    ms << msg_out;
    MessageData *msg_in = MsgDataFactory( ms.Data(), header, msg_out.protocol_version());
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
    util::MemoryStream ms;    
    Pong msg_out(0x1122334455667788);
    MessageHeader header(kMainnetMagic, msg_command::kMsgPong, msg_out.SerializedSize(),
                         msg_out.GetHash().GetLow32());
    
    ms << msg_out;
    MessageData *msg_in = MsgDataFactory(ms.Data(), header, 0);
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_out, *reinterpret_cast<Pong*>(msg_in));
    delete msg_in;
}

TEST(MsgFactoryTest, RejectFactory)
{
    util::MemoryStream ms;
    Reject msg_out(msg_command::kMsgVersion, CCode::kRejectDuplicate, 
                   "Duplicate version message", std::move(util::RandHash256()));
    MessageHeader header(kMainnetMagic, msg_command::kMsgReject, msg_out.SerializedSize(), 
                         msg_out.GetHash().GetLow32());
    
    ms << msg_out;    
    MessageData *msg_in = MsgDataFactory(ms.Data(), header, 0);
    ASSERT_NE(msg_in, nullptr);
    EXPECT_EQ(msg_out, *reinterpret_cast<Reject*>(msg_in));
    delete msg_in;
}

TEST(MsgFactoryTest, SendHeadersFactory)
{
    util::MemoryStream ms;
    SendHeaders send_headers;
    MessageHeader header(kMainnetMagic, msg_command::kMsgSendHeaders, send_headers.SerializedSize(), 
                         send_headers.GetHash().GetLow32());
    
    ms << send_headers;    
    MessageData *msg = MsgDataFactory(ms.Data(), header, 0);
    ASSERT_NE(msg, nullptr);
    EXPECT_EQ(msg->Command(), msg_command::kMsgSendHeaders);
    delete msg;
}

TEST(MsgFactoryTest, SendCmpctFactory)
{
    util::MemoryStream ms;
    SendCmpct send_compact(true, 1);
    MessageHeader header(kMainnetMagic, msg_command::kMsgSendCmpct, send_compact.SerializedSize(), 
                         send_compact.GetHash().GetLow32());
    
    ms << send_compact;    
    MessageData *msg = MsgDataFactory( ms.Data(), header, 0);
    ASSERT_NE(msg, nullptr);
    EXPECT_EQ(*reinterpret_cast<SendCmpct*>(msg), send_compact);
}

TEST(MsgFactoryTest, NullFactory)
{
    MessageHeader header(kMainnetMagic, msg_command::kMsgVersion, 0, 0);
    MessageData *msg_in = MsgDataFactory(nullptr, header, 0);
    EXPECT_EQ(msg_in, nullptr);
    
    uint8_t foo;
    header.set_command("bar");
    msg_in = MsgDataFactory(&foo, header, 0);
    EXPECT_EQ(msg_in, nullptr);
}


void ReadCb(struct bufferevent *bev, void *ctx)
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
    delete message;
    
    event_base_loopexit(base, NULL);
}

TEST_F(MsgProcessTest, SendVersion)
{
    ASSERT_NE(pair_[0], nullptr);
    ASSERT_NE(pair_[1], nullptr);
    
    bufferevent_setcb(pair_[1], ReadCb, NULL, NULL, const_cast<char*>(msg_command::kMsgVersion));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    node->set_services(kNodeNetwork);
    EXPECT_TRUE(SendVersion(node, kTestnetMagic));
    
    event_base_dispatch(base_);
    
    //bufferevent_free(pair_[0]);
    //bufferevent_free(pair_[1]);
    //event_base_free(base_);
}

TEST_F(MsgProcessTest, SendVerack)
{
    bufferevent_setcb(pair_[1], ReadCb, NULL, NULL, const_cast<char*>(msg_command::kMsgVerack));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    Verack verack;
    EXPECT_TRUE(SendMsg(verack, kTestnetMagic, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendAddr)
{
    bufferevent_setcb(pair_[1], ReadCb, NULL, NULL, const_cast<char*>(msg_command::kMsgAddr));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    NetAddr addr;
    Addr addr_msg;
    
    addr.SetIpv4(inet_addr("1.2.3.1"));
    addr_msg.mutable_addr_list()->push_back(addr);
    addr.SetIpv4(inet_addr("1.2.3.2"));
    addr_msg.mutable_addr_list()->push_back(addr);
    EXPECT_TRUE(SendMsg(addr_msg, kTestnetMagic, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendInv)
{
    bufferevent_setcb(pair_[1], ReadCb, NULL, NULL, const_cast<char*>(msg_command::kMsgInv));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    Inv inv;
    inv.mutable_inv_vects()->emplace_back(DataMsgType::kMsgTx, util::RandHash256());
    inv.mutable_inv_vects()->emplace_back(DataMsgType::kMsgBlock, util::RandHash256());
    EXPECT_TRUE(SendMsg(inv, kTestnetMagic, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendGetAddr)
{
    bufferevent_setcb(pair_[1], ReadCb, NULL, NULL, const_cast<char*>(msg_command::kMsgGetAddr));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    GetAddr getaddr;
    EXPECT_TRUE(SendMsg(getaddr, kTestnetMagic, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendPing)
{
    bufferevent_setcb(pair_[1], ReadCb, NULL, NULL, const_cast<char*>(msg_command::kMsgPing));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    Ping ping(util::RandUint64());
    EXPECT_TRUE(SendMsg(ping, kTestnetMagic, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendPong)
{
    bufferevent_setcb(pair_[1], ReadCb, NULL, NULL, const_cast<char*>(msg_command::kMsgPong));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    Pong pong(util::RandUint64());
    EXPECT_TRUE(SendMsg(pong, kTestnetMagic, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendSendHeaders)
{
    bufferevent_setcb(pair_[1], ReadCb, NULL, NULL, const_cast<char*>(msg_command::kMsgSendHeaders));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    SendHeaders send_headers;
    EXPECT_TRUE(SendMsg(send_headers, kTestnetMagic, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendSendCmpct)
{
    bufferevent_setcb(pair_[1], ReadCb, NULL, NULL, const_cast<char*>(msg_command::kMsgSendCmpct));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    SendCmpct send_compact(true, 1);
    EXPECT_TRUE(SendMsg(send_compact, kTestnetMagic, node));
    
    event_base_dispatch(base_);
}

TEST_F(MsgProcessTest, SendReject)
{
    bufferevent_setcb(pair_[1], ReadCb, NULL, NULL, const_cast<char*>(msg_command::kMsgReject));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    Reject reject(msg_command::kMsgVersion, CCode::kRejectDuplicate, 
                  "Duplicate version message", std::move(util::RandHash256()));
    EXPECT_TRUE(SendMsg(reject, kTestnetMagic, node));
    
    event_base_dispatch(base_);
}
#endif

void ParseMsgCb(struct bufferevent *bev, void *ctx)
{
    Peers peers;
    NetAddr addr;
    addr.SetIpv4(inet_addr("1.2.3.4"));
    auto node = std::make_shared<Node>(bev, addr, false);
    node->mutable_protocol()->version = kInvalidCbNoBanVersion;
    node->mutable_connection()->set_connection_state(NodeConnection::kEstablished);
    EXPECT_TRUE(ParseMsg(node, *reinterpret_cast<Params*>(ctx), LocalService(), &peers));
}

TEST_F(MsgProcessTest, ParseMsg)
{
    Params params(BtcNet::kTestNet, util::Args(), fs::path("/tmp/foo"));
    bufferevent_setcb(pair_[1], ParseMsgCb, NULL, NULL, reinterpret_cast<void*>(&params));
    bufferevent_enable(pair_[1], EV_READ);
    auto node = std::make_shared<Node>(pair_[0], addr_, false);
    Ping ping(util::RandUint64());
    EXPECT_TRUE(SendMsg(ping, kTestnetMagic, node));
    
    event_base_dispatch(base_);
}

} // namespace unit_test
} // namespace btclit
