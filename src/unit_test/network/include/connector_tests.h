#include "connector.h"

#include <gtest/gtest.h>

#include "banlist.h"
#include "net.h"
#include "peers.h"


namespace btclite {
namespace unit_test {

class ConnectorTest : public ::testing::Test {
protected:
    ConnectorTest()
        : connector_(), params_(BtcNet::kTestNet, util::Args(), fs::path("/tmp/foo")),
          local_service_(), chain_state_(), banlist_(), peers_(),
          ctx_({&params_, &local_service_, (network::Nodes*)&connector_.outbounds(), &chain_state_, &banlist_, &peers_}) {}
    
    network::Connector connector_;
    network::Params params_;
    network::LocalService local_service_;
    chain::ChainState chain_state_;
    network::BanList banlist_;
    network::Peers peers_;    
    network::Context ctx_;
    std::vector<network::NetAddr> addrs_;
};

} // namespace unit_test
} // namespace btclite
