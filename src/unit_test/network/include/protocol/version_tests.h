#include <gtest/gtest.h>

#include "network_address.h"
#include "protocol/version.h"


using namespace btclite::network::protocol;

class VersionTest : public ::testing::Test {
protected:
    VersionTest()
        : version_(kProtocolVersion), services_(btclite::network::kNodeNetwork), 
          timestamp_(0x1234),
          addr_recv_(0x1234, btclite::network::kNodeNetwork, 
                     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x4},
                     8333),
          addr_from_(0x5678, btclite::network::kNodeNetwork, 
                     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x1, 0x2, 0x3, 0x5},
                     8333),
          nonce_(0x5678), user_agent_(std::string("/btclite:0.1.0/")), 
          start_height_(1000), relay_(true),
          version1_(),
          version2_(version_, services_, timestamp_, addr_recv_, addr_from_, nonce_, 
                    user_agent_, start_height_, relay_),
          version3_(version_, services_, timestamp_,
                    std::move(btclite::network::NetAddr(addr_recv_)),
                    std::move(btclite::network::NetAddr(addr_from_)),
                    nonce_, std::move(std::string(user_agent_)),
                    start_height_, relay_) {}
    
    uint32_t version_ = 0;
    uint64_t services_ = 0;
    uint64_t timestamp_ = 0;
    btclite::network::NetAddr addr_recv_;
    btclite::network::NetAddr addr_from_;
    uint64_t nonce_ = 0;
    std::string user_agent_;
    uint32_t start_height_ = 0;
    bool relay_ = false;
    
    Version version1_;
    Version version2_;
    Version version3_;    
};
