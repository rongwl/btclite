#include <gtest/gtest.h>

#include "protocol/reject.h"
#include "random.h"


namespace btclite {
namespace unit_test {

class RejectTest : public ::testing::Test {
protected:
    RejectTest()
        : message_(msg_command::kMsgBlock), 
          ccode_(network::protocol::CCode::kRejectDuplicate), 
          reason_("foo"), data_(util::GetUint256()),
          reject1_(), reject2_(message_, ccode_, reason_, data_),
          reject3_(std::move(std::string(message_)), ccode_,
                   std::move(std::string(reason_)), data_) {}
    
    std::string message_;
    network::protocol::CCode ccode_ = network::protocol::CCode::kRejectUnknown;
    std::string reason_;
    crypto::Hash256 data_;
    
    network::protocol::Reject reject1_;
    network::protocol::Reject reject2_;
    network::protocol::Reject reject3_;
};

} // namespace unit_test
} // namespace btclit
