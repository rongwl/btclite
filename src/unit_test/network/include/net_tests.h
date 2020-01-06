#include <gtest/gtest.h>

#include "net.h"


namespace btclite {
namespace unit_test {

class LocalNetConfigTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        config_.LookupLocalAddrs();
    }
    
    network::LocalNetConfig config_;
};

} // namespace unit_test
} // namespace btclit
