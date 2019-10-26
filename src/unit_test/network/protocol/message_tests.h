#include <gtest/gtest.h>

#include "protocol/message.h"


class FixtureMessageTest : public ::testing::Test {
protected:
    FixtureMessageTest()
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
    
    MessageHeader header1_;
    MessageHeader header2_;
    MessageHeader header3_;
};
