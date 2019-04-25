#include <gtest/gtest.h>

#include "util_tests.h"

TEST(ArgsTest, MethordSetArg)
{
    TestArgs args;
    
    args.SetArg("test", "123");
    EXPECT_TRUE(args.IsArgSet("test"));
    EXPECT_EQ(args.GetArg("test", ""), "123");
    EXPECT_EQ(args.GetArg("Test", "default"), "default");
}

TEST(ArgsTest, MethordSetArgs)
{
    TestArgs args;
    std::vector<std::string> vs = { "1", "2", "3" };
    
    args.SetArgs("test", vs[0]);
    args.SetArgs("test", vs[1]);
    args.SetArgs("test", vs[2]);
    EXPECT_TRUE(args.IsArgSet("test"));
    EXPECT_EQ(args.GetArgs("test"), vs);
}
