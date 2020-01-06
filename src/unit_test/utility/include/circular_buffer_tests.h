#include <gtest/gtest.h>

#include "circular_buffer.h"


namespace btclite {
namespace unit_test {

class CircularBufferTest : public ::testing::Test {
protected:
    CircularBufferTest()
        : buf0_(3), buf1_(3), buf2_(3), buf3_(3) {}
    
    void SetUp() override
    {
        buf1_.push_back(1);
        
        buf2_.push_back(1);
        buf2_.push_back(2);
        
        buf3_.push_back(1);
        buf3_.push_back(2);
        buf3_.push_back(3);
    }
    
    util::CircularBuffer<int> buf0_;
    util::CircularBuffer<int> buf1_;
    util::CircularBuffer<int> buf2_;
    util::CircularBuffer<int> buf3_;
};

} // namespace unit_test
} // namespace btclit
