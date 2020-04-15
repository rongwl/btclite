#include <gtest/gtest.h>

#include "net.h"


namespace btclite {
namespace unit_test {

class LocalServiceTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        service_.DiscoverLocalAddrs();
    }
    
    network::LocalService service_;
};

} // namespace unit_test
} // namespace btclit
