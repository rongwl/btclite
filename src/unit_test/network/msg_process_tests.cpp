#include <gtest/gtest.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "msg_process.h"


using namespace btclite::network;

void ReadCb(struct bufferevent *bev, void *ctx)
{
    struct event_base *base = reinterpret_cast<struct event_base*>(ctx);
    NetAddr addr;
    
    addr.SetIpv4(inet_addr("1.2.3.5"));
    auto node = std::make_shared<Node>(bev, addr, false);
    EXPECT_TRUE(msgprocess::ParseMsg(node));
    
    event_base_loopexit(base, NULL);
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
