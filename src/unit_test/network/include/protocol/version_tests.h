#include <gtest/gtest.h>

#include "network_address.h"
#include "protocol/version.h"


namespace btclite {
namespace unit_test {

class VersionTest : public ::testing::Test {
protected:
    VersionTest()
        : addr_recv_(0x1234, network::kNodeNetwork, 
                     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x4},
                     8333),
          addr_from_(0x5678, network::kNodeNetwork, 
                     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x5},
                     8333),
          version1_(),
          version2_(version_, services_, timestamp_, addr_recv_, addr_from_, nonce_, 
                    user_agent_, start_height_, relay_),
          version3_(version_, services_, timestamp_,
                    std::move(network::NetAddr(addr_recv_)),
                    std::move(network::NetAddr(addr_from_)),
                    nonce_, std::move(std::string(user_agent_)),
                    start_height_, relay_) {}
    
    network::ProtocolVersion version_ = network::protocol::kProtocolVersion;
    network::ServiceFlags services_ = network::kNodeNetwork;
    uint64_t timestamp_ = 0x1234;
    network::NetAddr addr_recv_;
    network::NetAddr addr_from_;
    uint64_t nonce_ = 0x5678;
    std::string user_agent_ = "/btclite:0.1.0/";
    uint32_t start_height_ = 1000;
    bool relay_ = true;
    
    network::protocol::Version version1_;
    network::protocol::Version version2_;
    network::protocol::Version version3_;    
};

} // namespace unit_test
} // namespace btclit
