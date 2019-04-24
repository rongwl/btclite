#include <gtest/gtest.h>

#include "fullnode/include/config.h"
#include "error.h"

TEST(FullNodeArgsTest, MethordInit)
{
    FullNodeArgs args;
    std::string argv0 = "btc-fullnode";
    char *argv[3];
    char argv1[32] = {0}, argv2[32] = {0};    
    argv[0] = const_cast<char*>(argv0.c_str());
    argv[1] = argv1;
    argv[2] = argv2;
    
    std::strcpy(argv1, "-h");
    EXPECT_EXIT(args.Init(2, argv), ::testing::ExitedWithCode(ErrorCode::show_help), "");
    std::strcpy(argv1, "-?");
    EXPECT_EXIT(args.Init(2, argv), ::testing::ExitedWithCode(ErrorCode::show_help), "");
    std::strcpy(argv1, "--help");
    EXPECT_EXIT(args.Init(2, argv), ::testing::ExitedWithCode(ErrorCode::show_help), "");
    
    
    std::strcpy(argv1, "-debug");
    try {
        args.Init(2, argv);
        FAIL() << "expected ErrorCode::invalid_option";
    }
    catch (const Exception& e) {
        EXPECT_EQ(e.code().value(), ErrorCode::invalid_option);
    }
    catch (...) {
        FAIL() << "Expected ErrorCode::invalid_option";
    }
    
    std::strcpy(argv1, "--");
    try {
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

TEST(FullNodeArgsTest, MethordInitParameters)
{
    FullNodeArgs args;
    std::string argv0 = "btc-fullnode";
    char *argv[3];
    char argv1[32] = {0}, argv2[32] = {0}, argv3[32] = {0};  
    argv[0] = const_cast<char*>(argv0.c_str());
    argv[1] = argv1;
    argv[2] = argv2;
    argv[3] = argv3;
    
    std::strcpy(argv1, "--loglevel=6");
    optind = 1;
    try {
        args.Init(2, argv);
        args.InitParameters();
        FAIL() << "expected ErrorCode::invalid_argument";
    }
    catch (const Exception& e) {
        EXPECT_EQ(e.code().value(), ErrorCode::invalid_argument);
    }
    catch (...) {
        FAIL() << "Expected ErrorCode::invalid_argument";
    }
    
    std::strcpy(argv1, "--loglevel=a");
    optind = 1;
    try {
        args.Init(2, argv);
        args.InitParameters();
        FAIL() << "expected ErrorCode::invalid_argument";
    }
    catch (const Exception& e) {
        EXPECT_EQ(e.code().value(), ErrorCode::invalid_argument);
    }
    catch (...) {
        FAIL() << "Expected ErrorCode::invalid_argument";
    }
    
    std::strcpy(argv1, "--debug=123");
    optind = 1;
    try {        
        args.Init(2, argv);
        args.InitParameters();
        FAIL() << "expected ErrorCode::invalid_argument";
    }
    catch (const Exception& e) {
        EXPECT_EQ(e.code().value(), ErrorCode::invalid_argument);
    }
    catch (...) {
        FAIL() << "Expected ErrorCode::invalid_argument";
    }
    
    std::strcpy(argv1, "--testnet");
    std:;strcpy(argv2, "--regtest");
    optind = 1;
    try {
        args.Init(3, argv);
        args.InitParameters();
        FAIL() << "expected ErrorCode::invalid_option";
    }
    catch (const Exception& e) {
        EXPECT_EQ(e.code().value(), ErrorCode::invalid_option);
    }
    catch (...) {
        FAIL() << "Expected ErrorCode::invalid_option";
    }
    
    
    /*std::strcpy(argv1, "--debug=net");
    std::strcpy(argv1, "--debug=mempool");
    optind = 1;
    try {        
        args.Init(3, argv);
        args.InitParameters();
        EXPECT_EQ(Logging::log_module(), Logging::NET | Logging::MEMPOOL);
    }
    catch (...) {
        FAIL() << "Expected ErrorCode::invalid_argument";
    }*/
}
