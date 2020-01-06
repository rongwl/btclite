#include <gtest/gtest.h>

#include "protocol/send_compact.h"


namespace btclite {
namespace unit_test {

class SendCmpctTest : public ::testing::Test {
protected:
    SendCmpctTest()
        : send_compact1_(), send_compact2_(true, 1) {}
    
    network::protocol::SendCmpct send_compact1_;
    network::protocol::SendCmpct send_compact2_;
};

} // namespace unit_test
} // namespace btclit
