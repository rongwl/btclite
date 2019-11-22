#include <gtest/gtest.h>

#include "protocol/pong.h"


using namespace btclite::network::protocol;

class PongTest : public ::testing::Test {
protected:
    PongTest()
        : nonce_(0x1122334455667788), pong1_(), pong2_(nonce_) {}
    
    uint64_t nonce_ = 0;
    Pong pong1_;
    Pong pong2_;
};
