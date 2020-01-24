#include <gtest/gtest.h>

#include "protocol/ping.h"


namespace btclite {
namespace unit_test {

class PingTest : public ::testing::Test {
protected:
    PingTest()
        : ping1_(), ping2_(nonce_), ping3_(0, 0) {}
    
    uint64_t nonce_ = 0x1122334455667788;
    network::protocol::Ping ping1_;
    network::protocol::Ping ping2_;
    network::protocol::Ping ping3_;
};

} // namespace unit_test
} // namespace btclit
