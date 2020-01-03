#include <gtest/gtest.h>

#include "protocol/reject.h"
#include "random.h"


using namespace btclite::network::protocol;

class RejectTest : public ::testing::Test {
protected:
    RejectTest()
        : message_(::kMsgBlock), ccode_(CCode::kRejectDuplicate), reason_("foo"), 
          data_(btclite::utility::GetUint256()),
          reject1_(), reject2_(message_, ccode_, reason_, data_),
          reject3_(std::move(std::string(message_)), ccode_,
                   std::move(std::string(reason_)), data_) {}
    
    std::string message_;
    CCode ccode_ = CCode::kRejectUnknown;
    std::string reason_;
    Hash256 data_;
    
    Reject reject1_;
    Reject reject2_;
    Reject reject3_;
};
