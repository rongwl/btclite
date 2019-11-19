#include <gtest/gtest.h>

#include "protocol/send_compact.h"


using namespace btclite::network::protocol;

class SendCmpctTest : public ::testing::Test {
protected:
    SendCmpctTest()
        : send_compact1_(), send_compact2_(true, 1) {}
    
    SendCmpct send_compact1_;
    SendCmpct send_compact2_;
};
