#include <gtest/gtest.h>

#include "protocol/message.h"


namespace btclite {
namespace unit_test {

class MessageHeaderTest : public ::testing::Test {
protected:
    MessageHeaderTest()
        : magic_(kMainMagic), command_(kMsgVersion),
          payload_length_(kMaxMessageSize), checksum_(0x12345678),
          header1_(), 
          header2_(magic_, command_,
                   payload_length_, checksum_),
          header3_(magic_, std::move(std::string(command_)),
                   payload_length_, checksum_) {}
    
    uint32_t magic_ = 0;
    std::string command_;
    uint32_t payload_length_ = 0;
    uint32_t checksum_ = 0;
    
    network::protocol::MessageHeader header1_;
    network::protocol::MessageHeader header2_;
    network::protocol::MessageHeader header3_;
};

} // namespace unit_test
} // namespace btclite
