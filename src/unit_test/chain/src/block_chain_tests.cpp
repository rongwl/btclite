#include <gtest/gtest.h>

#include "block_chain.h"
#include "arithmetic.h"
#include "random.h"


namespace btclite {
namespace unit_test {

TEST(BlockChainTest, GetLocator)
{
    // Build a main chain 100000 blocks long.
    std::vector<util::Hash256> hash_main(100000);
    std::vector<chain::BlockIndex> blocks_main(100000);
    blocks_main[0].set_block_hash(hash_main[0]);
    for (uint32_t i = 1; i < blocks_main.size(); ++i) {
        hash_main[i] = util::Hash256(i); // Set the hash equal to the height, so we can quickly check the distances.
        blocks_main[i].set_height(i);
        blocks_main[i].set_prev(&blocks_main[i - 1]);
        blocks_main[i].set_block_hash(hash_main[i]);
        //EXPECT_EQ(blocks_main[i].block_hash().GetLow32(), blocks_main[i].height());
        //EXPECT_TRUE(blocks_main[i].prev() == nullptr || blocks_main[i].height() == blocks_main[i].prev()->height() + 1);
    }
    
    // Build a branch that splits off at block 49999, 50000 blocks long.
    std::vector<util::Hash256> hash_side(50000);
    std::vector<chain::BlockIndex> blocks_side(50000);
    for (uint32_t i = 0; i < blocks_side.size(); ++i) {
        hash_side[i] = util::Hash256(i + 50000 + (1UL << 32));// Add 1<<32 to the hashes, so GetLow32() still returns the height.
        blocks_side[i].set_height(i + 50000);
        blocks_side[i].set_prev((i ? &blocks_side[i - 1] : &blocks_main[49999]));
        blocks_side[i].set_block_hash(hash_side[i]);
        //EXPECT_EQ(blocks_side[i].block_hash().GetLow32(), blocks_side[i].height());
        //EXPECT_EQ(blocks_side[i].height(), blocks_side[i].prev()->height() + 1);
    }
    
    // Build a Chain for the main branch.
    chain::Chain chain;
    chain.SetTip(&blocks_main.back());
    
    // Test 100 random starting points for locators.
    for (int n = 0; n < 100; ++n) {
        uint32_t r = util::RandUint64(150000);
        chain::BlockIndex *tip = (r < 100000) ? &blocks_main[r] : &blocks_side[r - 100000];
        consensus::BlockLocator locator;
        
        ASSERT_TRUE(chain.GetLocator(&locator, tip));

        // The first result must be the block itself, the last one must be genesis.
        EXPECT_EQ(locator.front(), tip->block_hash());
        EXPECT_EQ(locator.back(), blocks_main[0].block_hash());

        // Entries 1 through 11 (inclusive) go back one step each.
        for (uint32_t i = 1; i < 12 && i < locator.size() - 1; ++i) {
            EXPECT_EQ(locator[i].GetLow32(), tip->height() - i);
        }

        // The further ones (excluding the last one) go back with exponential steps.
        uint32_t dist = 2;
        for (uint32_t i = 12; i < locator.size() - 1; ++i) {
            EXPECT_EQ(locator[i-1].GetLow32() - locator[i].GetLow32(), dist);
            dist *= 2;
        }
    }

}

} // namespace unit_test
} // namespace btclite
