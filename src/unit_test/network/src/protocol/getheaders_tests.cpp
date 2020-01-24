#include <gtest/gtest.h>

#include "protocol/getheaders.h"


namespace btclite {
namespace unit_test {

TEST(GetHeadersTest, Command)
{
    network::protocol::GetHeaders getheaders;
    EXPECT_EQ(getheaders.Command(), msg_command::kMsgGetHeaders);
}

} // namespace unit_test
} // namespace btclit
