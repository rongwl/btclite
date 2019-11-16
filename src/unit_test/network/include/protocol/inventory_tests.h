#include <gtest/gtest.h>

#include "protocol/inventory.h"
#include "random.h"


using namespace btclite::network::protocol;

class InvTest : public ::testing::Test {
protected:
    InvTest()
        : inv1_(), inv2_()
    {
        inv2_.mutable_inv_vects()->emplace_back(DataMsgType::kMsgTx,
                                                btclite::utility::random::GetUint256());
        inv2_.mutable_inv_vects()->emplace_back(DataMsgType::kMsgBlock, 
                                                btclite::utility::random::GetUint256());
    }
    
    Inv inv1_;
    Inv inv2_;
};
