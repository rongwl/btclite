#include <gtest/gtest.h>

#include "protocol/inventory.h"
#include "random.h"


namespace btclite {
namespace unit_test {

class InvTest : public ::testing::Test {
protected:
    InvTest()
        : inv1_(), inv2_()
    {
        inv2_.mutable_inv_vects()->emplace_back(network::protocol::DataMsgType::kMsgTx,
                                                util::RandHash256());
        inv2_.mutable_inv_vects()->emplace_back(network::protocol::DataMsgType::kMsgBlock, 
                                                util::RandHash256());
    }
    
    network::protocol::Inv inv1_;
    network::protocol::Inv inv2_;
};

} // namespace unit_test
} // namespace btclit
