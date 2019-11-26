#include <gtest/gtest.h>

#include "net.h"


class LocalNetConfigTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        config_.LookupLocalAddrs();
    }
    
    btclite::network::LocalNetConfig config_;
};
