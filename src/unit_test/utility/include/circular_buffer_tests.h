#include <gtest/gtest.h>

#include "circular_buffer.h"


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
    
    CircularBuffer<int> buf0_;
    CircularBuffer<int> buf1_;
    CircularBuffer<int> buf2_;
    CircularBuffer<int> buf3_;
};
