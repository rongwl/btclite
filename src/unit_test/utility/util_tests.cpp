#include <gtest/gtest.h>

#include "util_tests.h"

TEST(ArgsTest, MethordSetArg)
{
    TestArgs args;
    
    args.SetArg("test", "123");
    ASSERT_TRUE(args.IsArgSet("test"));
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
    ASSERT_TRUE(args.IsArgSet("test"));
    EXPECT_EQ(args.GetArgs("test"), vs);
}

TEST(ArgsTest, MethordGetBoolArg)
{
    TestArgs args;
    
    args.SetArg("test", "1");
    EXPECT_TRUE(args.GetBoolArg("test", false));
    args.SetArg("test", "0");
    EXPECT_FALSE(args.GetBoolArg("test", true));
    args.SetArg("test", "foo");
    EXPECT_TRUE(args.GetBoolArg("test", false));
}

TEST(DataFilesTest, Constructor)
{
    TestDataFiles data_files1;
    EXPECT_EQ(data_files1.path_data_dir(), DataFiles::PathHome());
    EXPECT_EQ(data_files1.path_config_file(), data_files1.path_data_dir() / "btclite.conf");
    
    TestDataFiles data_files2(fs::path("/foo"));
    EXPECT_EQ(data_files2.path_data_dir(), fs::path("/foo"));
    EXPECT_EQ(data_files2.path_config_file(), fs::path("/foo") / "btclite.conf");
    
    TestDataFiles data_files3(fs::path("/foo"), "bar.conf");
    EXPECT_EQ(data_files3.path_data_dir(), fs::path("/foo"));
    EXPECT_EQ(data_files3.path_config_file(), fs::path("/foo") / "bar.conf");
}
