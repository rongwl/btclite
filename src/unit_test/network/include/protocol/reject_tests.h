#include <gtest/gtest.h>

#include "protocol/reject.h"
#include "random.h"


using namespace btclite::network::protocol;

class RejectTest : public ::testing::Test {
protected:
    RejectTest()
        : message_(kMsgBlock), ccode_(kRejectDuplicate), reason_("foo"), 
          data_(btclite::utility::random::GetUint256()),
          reject1_(), reject2_(message_, ccode_, reason_, data_),
          reject3_(std::move(std::string(message_)), ccode_,
                   std::move(std::string(reason_)), data_) {}
    
    std::string message_;
    uint8_t ccode_ = 0;
    std::string reason_;
    Hash256 data_;
    
    Reject reject1_;
    Reject reject2_;
    Reject reject3_;
};
