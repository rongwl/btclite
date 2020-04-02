#include <gtest/gtest.h>

#include "protocol/getblocks.h"
#include "random.h"


namespace btclite {
namespace unit_test {

class GetBlocksTest : public ::testing::Test {
protected:
    GetBlocksTest()
        : getblocks1_(),
          getblocks2_(version_, hashes_, hash_stop_),
          getblocks3_(version_, std::move(chain::BlockLocator(hashes_)), hash_stop_) {}
    
    network::ProtocolVersion version_ = network::kInvalidCbNoBanVersion;
    chain::BlockLocator hashes_{ util::RandHash256(), util::RandHash256() };
    util::Hash256 hash_stop_ = util::RandHash256();
    network::protocol::GetBlocks getblocks1_;
    network::protocol::GetBlocks getblocks2_;
    network::protocol::GetBlocks getblocks3_;
};

} // namespace unit_test
} // namespace btclit
