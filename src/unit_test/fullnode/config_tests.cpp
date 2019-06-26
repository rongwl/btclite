#include <gtest/gtest.h>

#include "fullnode/include/config.h"
#include "error.h"
#include "utility/include/logging.h"


#if 0
TEST(FullNodeArgsTest, OptionHelp)
{
    FullNodeArgs args;
    std::string argv0 = "btc-fullnode";
    
    optind = 1;
    try {
        char *argv[2];
        std::string argv1 = "--help";
        
        argv[0] = const_cast<char*>(argv0.c_str());
        argv[1] = const_cast<char*>(argv1.c_str());
        args.Clear();
        args.Init(2, argv);
        FAIL() << "expected ErrorCode::show_help";
    }
    catch (const Exception& e) {
        EXPECT_EQ(e.code().value(), ErrorCode::show_help);
    }
    catch (...) {
        FAIL() << "Expected ErrorCode::show_help";
    }

    optind = 1;
    try {
        char *argv[2];
        std::string argv1 = "-h";
        
        argv[0] = const_cast<char*>(argv0.c_str());
        argv[1] = const_cast<char*>(argv1.c_str());
        args.Clear();
        args.Init(2, argv);
        FAIL() << "expected ErrorCode::show_help";
    }
    catch (const Exception& e) {
        EXPECT_EQ(e.code().value(), ErrorCode::show_help);
    }
    catch (...) {
        FAIL() << "Expected ErrorCode::show_help";
    }
    
    optind = 1;
    try {
        char *argv[2];
        std::string argv1 = "-?";
        
        argv[0] = const_cast<char*>(argv0.c_str());
        argv[1] = const_cast<char*>(argv1.c_str());
        args.Clear();
        args.Init(2, argv);
        FAIL() << "expected ErrorCode::show_help";
    }
    catch (const Exception& e) {
        EXPECT_EQ(e.code().value(), ErrorCode::show_help);
    }
    catch (...) {
        FAIL() << "Expected ErrorCode::show_help";
    }
}

TEST(FullNodeArgsTest, OptionLog)
{
    FullNodeArgs args;
    std::string argv0 = "btc-fullnode";
    
    optind = 1;
    try {
        char *argv[4];        
        std::string argv1 = "--debug=mempool", argv2 = "--debug=net", argv3 = "--loglevel=5";
        
        argv[0] = const_cast<char*>(argv0.c_str());
        argv[1] = const_cast<char*>(argv1.c_str());
        argv[2] = const_cast<char*>(argv2.c_str());
        argv[3] = const_cast<char*>(argv3.c_str());
        
        args.Init(4, argv);
        args.InitParameters();
        EXPECT_TRUE(args.IsArgSet(GLOBAL_OPTION_DEBUG));
        EXPECT_EQ(Logging::log_module(), Logging::NET | Logging::MEMPOOL);
        EXPECT_TRUE(args.IsArgSet(GLOBAL_OPTION_LOGLEVEL));
        EXPECT_EQ(args.GetArg(GLOBAL_OPTION_LOGLEVEL, "4"), "5");
    }
    catch (const Exception& e) {
        FAIL() << "Exception:" << e.code().message();
    }
    catch (...) {
        FAIL() << "Exception";
    }
    
    optind = 1;
    try {
        char *argv[2];
        std::string argv1 = "--loglevel=-1";
        
        argv[0] = const_cast<char*>(argv0.c_str());
        argv[1] = const_cast<char*>(argv1.c_str());
        args.Clear();
        args.Init(2, argv);
        FAIL() << "expected ErrorCode::invalid_argument";
    }
    catch (const Exception& e) {
        EXPECT_EQ(e.code().value(), ErrorCode::invalid_argument);
    }
    catch (...) {
        FAIL() << "Expected ErrorCode::invalid_argument";
    }
    
    optind = 1;
    try {
        char *argv[2];
        std::string argv1 = "--loglevel=6";
        
        argv[0] = const_cast<char*>(argv0.c_str());
        argv[1] = const_cast<char*>(argv1.c_str());
        args.Clear();
        args.Init(2, argv);
        FAIL() << "expected ErrorCode::invalid_argument";
    }
    catch (const Exception& e) {
        EXPECT_EQ(e.code().value(), ErrorCode::invalid_argument);
    }
    catch (...) {
        FAIL() << "Expected ErrorCode::invalid_argument";
    }
    
    optind = 1;
    try {
        char *argv[2];
        std::string argv1 = "--loglevel=a";
        
        argv[0] = const_cast<char*>(argv0.c_str());
        argv[1] = const_cast<char*>(argv1.c_str());
        args.Clear();
        args.Init(2, argv);
        FAIL() << "expected ErrorCode::invalid_argument";
    }
    catch (const Exception& e) {
        EXPECT_EQ(e.code().value(), ErrorCode::invalid_argument);
    }
    catch (...) {
        FAIL() << "Expected ErrorCode::invalid_argument";
    }
    
    optind = 1;
    try {
        char *argv[2];
        std::string argv1 = "--debug=123";
        
        argv[0] = const_cast<char*>(argv0.c_str());
        argv[1] = const_cast<char*>(argv1.c_str());
        args.Clear();
        args.Init(2, argv);
        FAIL() << "expected ErrorCode::invalid_argument";
    }
    catch (const Exception& e) {
        EXPECT_EQ(e.code().value(), ErrorCode::invalid_argument);
    }
    catch (...) {
        FAIL() << "Expected ErrorCode::invalid_argument";
    }
}

TEST(FullNodeArgsTest, OptionChain)
{
    FullNodeArgs args;
    std::string argv0 = "btc-fullnode";
    
    optind = 1;
    try {
        char *argv[2];
        std::string argv1 = "--testnet";
        
        argv[0] = const_cast<char*>(argv0.c_str());
        argv[1] = const_cast<char*>(argv1.c_str());
        args.Clear();
        args.Init(2, argv);
        
        EXPECT_TRUE(args.IsArgSet(GLOBAL_OPTION_TESTNET));
    }
    catch (const Exception& e) {
        FAIL() << "Exception:" << e.code().message();
    }
    catch (...) {
        FAIL() << "Exception";
    }
    
    optind = 1;
    try {
        char *argv[2];
        std::string argv1 = "--regtest";
        
        argv[0] = const_cast<char*>(argv0.c_str());
        argv[1] = const_cast<char*>(argv1.c_str());
        args.Clear();
        args.Init(2, argv);
        
        EXPECT_TRUE(args.IsArgSet(GLOBAL_OPTION_REGTEST));
    }
    catch (const Exception& e) {
        FAIL() << "Exception:" << e.code().message();
    }
    catch (...) {
        FAIL() << "Exception";
    }
    
    optind = 1;
    try {
        char *argv[3];
        std::string argv1 = "--testnet", argv2 = "--regtest";
        
        argv[0] = const_cast<char*>(argv0.c_str());
        argv[1] = const_cast<char*>(argv1.c_str());
        argv[2] = const_cast<char*>(argv2.c_str());
        args.Clear();
        args.Init(3, argv);
        FAIL() << "expected ErrorCode::invalid_option";
    }
    catch (const Exception& e) {
        EXPECT_EQ(e.code().value(), ErrorCode::invalid_option);
    }
    catch (...) {
        FAIL() << "Expected ErrorCode::invalid_option";
    }
}

TEST(FullNodeArgsTest, OptionFiles)
{
    FullNodeArgs args;
    std::string argv0 = "btc-fullnode";
    
    optind = 1;
    try {
        char *argv[3];
        std::string argv1 = "--datadir=/foo", argv2 = "--conf=foo.conf";
        
        argv[0] = const_cast<char*>(argv0.c_str());
        argv[1] = const_cast<char*>(argv1.c_str());
        argv[2] = const_cast<char*>(argv2.c_str());
        args.Clear();
        args.Init(3, argv);
        
        EXPECT_TRUE(args.IsArgSet(GLOBAL_OPTION_DATADIR));
        EXPECT_EQ(args.GetArg(GLOBAL_OPTION_DATADIR, ""), "/foo");
        EXPECT_TRUE(args.IsArgSet(GLOBAL_OPTION_CONF));
        EXPECT_EQ(args.GetArg(GLOBAL_OPTION_CONF, ""), "foo.conf");
    }
    catch (const Exception& e) {
        FAIL() << "Exception:" << e.code().message();
    }
    catch (...) {
        FAIL() << "Exception";
    }
}

TEST(FullNodeArgsTest, OptionConnection)
{
    FullNodeArgs args;
    std::string argv0 = "btc-fullnode"; 
    
    optind = 1;
    try {
        char *argv[2];
        std::string argv1 = "--listen=0";
        
        argv[0] = const_cast<char*>(argv0.c_str());
        argv[1] = const_cast<char*>(argv1.c_str());
        args.Clear();
        args.Init(2, argv);
        args.InitParameters();
        
        EXPECT_TRUE(args.IsArgSet(FULLNODE_OPTION_LISTEN));
        EXPECT_EQ(args.GetArg(FULLNODE_OPTION_LISTEN, "1"), "0");
        EXPECT_TRUE(args.IsArgSet(FULLNODE_OPTION_DISCOVER));
        EXPECT_EQ(args.GetArg(FULLNODE_OPTION_DISCOVER, "1"), "0");
    }
    catch (const Exception& e) {
        FAIL() << "Exception:" << e.code().message();
    }
    catch (...) {
        FAIL() << "Exception";
    }
    
    optind = 1;
    try {
        char *argv[2];
        std::string argv1 = "--listen=2";
        
        argv[0] = const_cast<char*>(argv0.c_str());
        argv[1] = const_cast<char*>(argv1.c_str());
        args.Clear();
        args.Init(2, argv);
        FAIL() << "expected ErrorCode::invalid_argument";
    }
    catch (const Exception& e) {
        EXPECT_EQ(e.code().value(), ErrorCode::invalid_argument);
    }
    catch (...) {
        FAIL() << "expected ErrorCode::invalid_argument";
    }
    
    optind = 1;
    try {
        char *argv[2];
        std::string argv1 = "--connect=1.1.1.1111";
        
        argv[0] = const_cast<char*>(argv0.c_str());
        argv[1] = const_cast<char*>(argv1.c_str());
        args.Clear();
        args.Init(2, argv);
        FAIL() << "expected ErrorCode::invalid_argument";
    }
    catch (const Exception& e) {
        EXPECT_EQ(e.code().value(), ErrorCode::invalid_argument);
    }
    catch (...) {
        FAIL() << "expected ErrorCode::invalid_argument";
    }
    
    optind = 1;
    try {
        char *argv[3];
        std::vector<std::string> ip = { "1.1.1.1", "2.2.2.2" };
        std::string argv1 = "--connect="+ip[0], argv2 = "--connect="+ip[1];
        
        argv[0] = const_cast<char*>(argv0.c_str());
        argv[1] = const_cast<char*>(argv1.c_str());
        argv[2] = const_cast<char*>(argv2.c_str());
        args.Clear();
        args.Init(3, argv);
        args.InitParameters();
        
        EXPECT_TRUE(args.IsArgSet(FULLNODE_OPTION_CONNECT));
        EXPECT_EQ(args.GetArgs(FULLNODE_OPTION_CONNECT), ip);
        EXPECT_TRUE(args.IsArgSet(FULLNODE_OPTION_LISTEN));
        EXPECT_EQ(args.GetArg(FULLNODE_OPTION_LISTEN, "1"), "0");
        EXPECT_TRUE(args.IsArgSet(FULLNODE_OPTION_DNSSEED));
        EXPECT_EQ(args.GetArg(FULLNODE_OPTION_DNSSEED, "1"), "0");
        EXPECT_TRUE(args.IsArgSet(FULLNODE_OPTION_DISCOVER));
        EXPECT_EQ(args.GetArg(FULLNODE_OPTION_DISCOVER, "1"), "0");
    }
    catch (const Exception& e) {
        FAIL() << "Exception:" << e.code().message();
    }
    catch (...) {
        FAIL() << "Exception";
    }
}

TEST(FullNodeArgsTest, InvalidOption)
{
    FullNodeArgs args;
    std::string argv0 = "btc-fullnode";
    
    optind = 1;
    try {
        char *argv[2];
        std::string argv1 = "-debug";
        
        argv[0] = const_cast<char*>(argv0.c_str());
        argv[1] = const_cast<char*>(argv1.c_str());        
        args.Clear();
        args.Init(2, argv);
        FAIL() << "expected ErrorCode::invalid_option";
    }
    catch (const Exception& e) {
        EXPECT_EQ(e.code().value(), ErrorCode::invalid_option);
    }
    catch (...) {
        FAIL() << "Expected ErrorCode::invalid_option";
    }
    
    optind = 1;
    try {
        char *argv[2];
        std::string argv1 = "--";
        
        argv[0] = const_cast<char*>(argv0.c_str());
        argv[1] = const_cast<char*>(argv1.c_str());        
        args.Clear();
        args.Init(2, argv);
        FAIL() << "expected ErrorCode::invalid_option";
    }
    catch (const Exception& e) {
        EXPECT_EQ(e.code().value(), ErrorCode::invalid_option);
    }
    catch (...) {
        FAIL() << "Expected ErrorCode::invalid_option";
    }
}

TEST(FullNodeDataFilesTest, Conctructor)
{
    FullNodeDataFiles data_files;
    EXPECT_EQ(data_files.path_data_dir(), FullNodeDataFiles::DefaultDataDirPath());
    EXPECT_EQ(data_files.path_config_file(), data_files.path_data_dir() / DEFAULT_CONFIG_FILE);
}

TEST(FullNodeDataFilesTest, MethordInit)
{
    FullNodeDataFiles data_files;
    data_files.Init(fs::path("/123"), "bar.conf");
    EXPECT_EQ(data_files.path_data_dir(), FullNodeDataFiles::DefaultDataDirPath());
    EXPECT_EQ(data_files.path_config_file(), data_files.path_data_dir() / DEFAULT_CONFIG_FILE);
    
    data_files.Init();
    EXPECT_EQ(data_files.path_data_dir(), FullNodeDataFiles::DefaultDataDirPath());
    EXPECT_EQ(data_files.path_config_file(), data_files.path_data_dir() / DEFAULT_CONFIG_FILE);
    EXPECT_TRUE(fs::is_directory(data_files.path_data_dir()));
    std::ifstream ifs(data_files.path_config_file());
    EXPECT_TRUE(ifs.good());
    
    std::ofstream ofs(fs::path("/tmp") / "foo.conf");
    data_files.Init(fs::path("/tmp"), "foo.conf");
    EXPECT_EQ(data_files.path_data_dir(), fs::path("/tmp"));
    EXPECT_EQ(data_files.path_config_file(), fs::path("/tmp") / "foo.conf");
}
#endif
