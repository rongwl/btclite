#include "acceptor.h"

#include <gtest/gtest.h>
#include <arpa/inet.h>

#include "banlist.h"


namespace btclite {
namespace unit_test {

class AcceptorTest : public ::testing::Test {
protected:
    AcceptorTest()
        : acceptor_(18333), params_(BtcNet::kTestNet, util::Args(), fs::path("/tmp/foo")),
          ban_list_(),
          ctx_({nullptr, nullptr, &acceptor_.Inbounds(), nullptr, &ban_list_, nullptr}) {}
    
    void SetUp() override
    {
        memset(&client_addr_, 0, sizeof(client_addr_));
        client_addr_.sin6_family = AF_INET6;
        client_addr_.sin6_port = htons(18333);
    }
    
    network::Acceptor acceptor_;
    struct event_base *base_;
    struct evconnlistener *listener_;
    struct bufferevent *bev_;
    network::Socket::Fd fd_;
    network::Params params_;
    network::LocalService local_service_;
    chain::ChainState chain_state_;
    network::BanList ban_list_;
    network::Peers peers_;
    network::Context ctx_;
    
    struct sockaddr_in6 client_addr_;
};

} // namespace unit_test
} // namespace btclite
