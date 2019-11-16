#include <gtest/gtest.h>

#include "protocol/inventory_vector.h"
#include "random.h"


using namespace btclite::network::protocol;

class InvVectTest : public ::testing::Test {
protected:
    InvVectTest()
        : type_(DataMsgType::kMsgTx), hash_(btclite::utility::random::GetUint256()),
          inv_vect1_(), inv_vect2_(type_, hash_) {}
    
    DataMsgType type_;
    Hash256 hash_;
    InvVect inv_vect1_;
    InvVect inv_vect2_;
};
