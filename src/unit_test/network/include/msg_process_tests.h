#include <gtest/gtest.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "btcnet.h"
#include "constants.h"
#include "network_address.h"
#include "network/include/params.h"


namespace btclite {
namespace unit_test {

class MsgProcessTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        base_ = event_base_new();
        if (!base_)
            return;
        
        if (bufferevent_pair_new(base_, BEV_OPT_CLOSE_ON_FREE, pair_) != 0)
            return;
        
        addr_.SetIpv4(inet_addr("1.2.3.4"));
    }
    
    struct event_base *base_ = nullptr;
    struct bufferevent *pair_[2] = {};
    network::NetAddr addr_;
};


} // namespace unit_test
} // namespace btclit
