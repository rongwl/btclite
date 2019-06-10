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
    TestDataFiles data_files(fs::path("/foo"), "bar.conf");
    EXPECT_EQ(data_files.path_data_dir(), fs::path("/foo"));
    EXPECT_EQ(data_files.path_config_file(), fs::path("/foo") / "bar.conf");
}

TEST(DataFilesTest, MethordGetSet)
{
    TestDataFiles data_files;
    data_files.set_path_data_dir(fs::path("/tmp"));
    EXPECT_EQ(data_files.path_data_dir(), fs::path("/tmp"));
    
    std::ofstream file(fs::path("/tmp") / "foo.conf");
    data_files.set_path_config_file("foo.conf");
    EXPECT_EQ(data_files.path_config_file(), fs::path("/tmp") / "foo.conf");
    
    data_files.set_path_data_dir(fs::path("/123"));
    EXPECT_EQ(data_files.path_data_dir(), fs::path("/tmp"));
    
    data_files.set_path_config_file("bar.conf");
    EXPECT_EQ(data_files.path_config_file(), fs::path("/tmp") / "foo.conf");
}
