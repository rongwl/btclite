#include <gtest/gtest.h>

#include "protocol/ping.h"


using namespace btclite::network::protocol;

class FixturePingTest : public ::testing::Test {
protected:
    FixturePingTest()
        : nonce_(0x1122334455667788), ping1_(), ping2_(nonce_) {}
    
    uint64_t nonce_ = 0;
    Ping ping1_;
    Ping ping2_;
};
