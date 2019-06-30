#include <gtest/gtest.h>

#include "util_tests.h"

TEST(ArgsTest, MethodSetArg)
{
    Args args;
    
    args.SetArg("test", "123");
    ASSERT_TRUE(args.IsArgSet("test"));
    EXPECT_EQ(args.GetArg("test", ""), "123");
    EXPECT_EQ(args.GetArg("Test", "default"), "default");
}

TEST(ArgsTest, MethodSetArgs)
{
    Args args;
    std::vector<std::string> vs = { "1", "2", "3" };
    
    args.SetArgs("test", vs[0]);
    args.SetArgs("test", vs[1]);
    args.SetArgs("test", vs[2]);
    ASSERT_TRUE(args.IsArgSet("test"));
    EXPECT_EQ(args.GetArgs("test"), vs);
}

TEST(ArgsTest, MethodGetBoolArg)
{
    Args args;
    
    args.SetArg("test", "1");
    EXPECT_TRUE(args.GetBoolArg("test", false));
    args.SetArg("test", "0");
    EXPECT_FALSE(args.GetBoolArg("test", true));
    args.SetArg("test", "foo");
    EXPECT_TRUE(args.GetBoolArg("test", false));
}


