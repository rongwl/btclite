#include <gtest/gtest.h>

#include "protocol/pong.h"


namespace btclite {
namespace unit_test {

class PongTest : public ::testing::Test {
protected:
    PongTest()
        : pong1_(), pong2_(nonce_) {}
    
    uint64_t nonce_ = 0x1122334455667788;
    network::protocol::Pong pong1_;
    network::protocol::Pong pong2_;
};

} // namespace unit_test
} // namespace btclit
