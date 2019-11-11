#include <gtest/gtest.h>

#include "util.h"


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

TEST(MedianFilterTest, Constructor)
{
    MedianFilter<int> filter(5, 15);
    EXPECT_EQ(filter.sorted()[0], 15);
}

TEST(MedianFilterTest, MethordMedian)
{
    MedianFilter<int> filter(5, 15);
    
    EXPECT_EQ(filter.Median(), 15);

    filter.Input(20); // [15 20]
    EXPECT_EQ(filter.Median(), 17);

    filter.Input(30); // [15 20 30]
    EXPECT_EQ(filter.Median(), 20);

    filter.Input(3); // [3 15 20 30]
    EXPECT_EQ(filter.Median(), 17);

    filter.Input(7); // [3 7 15 20 30]
    EXPECT_EQ(filter.Median(), 15);

    filter.Input(18); // [3 7 18 20 30]
    EXPECT_EQ(filter.Median(), 18);

    filter.Input(0); // [0 3 7 18 30]
    EXPECT_EQ(filter.Median(), 7);
}


