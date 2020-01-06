#include <gtest/gtest.h>

#include "protocol/inventory_vector.h"
#include "random.h"


namespace btclite {
namespace unit_test {

class InvVectTest : public ::testing::Test {
protected:
    InvVectTest()
        : type_(network::protocol::DataMsgType::kMsgTx), 
          hash_(util::GetUint256()),
          inv_vect1_(), inv_vect2_(type_, hash_) {}
    
    network::protocol::DataMsgType type_;
    crypto::Hash256 hash_;
    network::protocol::InvVect inv_vect1_;
    network::protocol::InvVect inv_vect2_;
};

} // namespace unit_test
} // namespace btclit
