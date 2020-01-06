#include "util.h"

#include <algorithm>
#include <getopt.h>
#include <locale>
#include <stdarg.h>

#if defined(HAVE_CONFIG_H)
#include "config/btclite-config.h"
#endif

#include "error.h"


namespace btclite {
namespace util {

volatile std::sig_atomic_t SigMonitor::received_signal_ = 0;


static void HandleAllocFail()
{
    // Rather than throwing std::bad-alloc if allocation fails, terminate
    // immediately to (try to) avoid chain corruption.
    std::set_new_handler(std::terminate);
    BTCLOG(LOG_LEVEL_ERROR) << "Critical error: out of memory. Terminating.";
    
    // The log was successful, terminate now.
    std::terminate();
}

void HelpInfo::PrintUsage()
{
    fprintf(stdout, "Common Options:\n");
    fprintf(stdout, "  -h or -?,  --help     print this help message and exit\n");
    fprintf(stdout, "  --debug=<module>      output debugging information(default: 0)\n");
    fprintf(stdout, "                        <module> can be 1(output all debugging information),\n");
    fprintf(stdout, "                        mempool, net\n");
    fprintf(stdout, "  --loglevel=<level>    specify the type of message being printed(default: %s)\n", DEFAULT_LOG_LEVEL);
    fprintf(stdout, "                        <level> can be:\n");
    fprintf(stdout, "                            0(A fatal condition),\n");
    fprintf(stdout, "                            1(An error has occurred),\n");
    fprintf(stdout, "                            2(A warning),\n");
    fprintf(stdout, "                            3(Normal message),\n");
    fprintf(stdout, "                            4(Debug information),\n");
    fprintf(stdout, "                            5(Verbose information\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "Chain selection options:\n");
    fprintf(stdout, "  --testnet             Use the test chain\n");
    fprintf(stdout, "  --regtest             Enter regression test mode, which uses a special chain\n");
    fprintf(stdout, "                        in which blocks can be solved instantly. This is intended\n");
    fprintf(stdout, "                        for regression testing tools and app development.\n");
    
    //              "                                                                                "

}

void ExecutorConfig::CheckArguments() const
{
    // --testnet and --regtest
    if (args_.IsArgSet(GLOBAL_OPTION_TESTNET) && args_.IsArgSet(GLOBAL_OPTION_REGTEST)) 
        throw Exception(ErrorCode::invalid_option, "invalid combination of --testnet and --regtest");
    
    // --debug
    if (args_.IsArgSet(GLOBAL_OPTION_DEBUG)) {
        const std::vector<std::string> arg_values = args_.GetArgs(GLOBAL_OPTION_DEBUG);
        if (std::none_of(arg_values.begin(), arg_values.end(),
        [](const std::string& val) { return val == "0"; })) {
            auto result = std::find_if(arg_values.begin(), arg_values.end(), 
            [](const std::string& module) { return Logging::MapIntoModule(module) == Logging::NONE; });
            if (result != arg_values.end())
                throw Exception(ErrorCode::invalid_argument, "invalid module '" + *result + "'");
        }
    }
    
    // --loglevel
    if (args_.IsArgSet(GLOBAL_OPTION_LOGLEVEL)) {
        const std::string arg_val = args_.GetArg(GLOBAL_OPTION_LOGLEVEL, DEFAULT_LOG_LEVEL);
        int level = -1;
        try {
            level = std::stoi(arg_val);
        }
        catch (const std::exception& e) {
            throw Exception(ErrorCode::invalid_argument, "invalid loglevel '" + arg_val + "'");
        }
        if (level < 0 || level >= LOG_LEVEL_MAX) {
            throw Exception(ErrorCode::invalid_argument, "invalid loglevel '" + arg_val + "'");
        }
    }
}

bool ExecutorConfig::InitParameters()
{    
    // --debug
    if (args_.IsArgSet(GLOBAL_OPTION_DEBUG)) {
        const std::vector<std::string> arg_values = args_.GetArgs(GLOBAL_OPTION_DEBUG);
        if (std::none_of(arg_values.begin(), arg_values.end(),
        [](const std::string& val) { return val == "0"; })) {
            for (auto module : arg_values) {
                Logging::set_logModule(Logging::MapIntoModule(module));
            }
        }
    }
    
    // --loglevel
    if (args_.IsArgSet(GLOBAL_OPTION_LOGLEVEL)) {
        const std::string arg_val = args_.GetArg(GLOBAL_OPTION_LOGLEVEL, DEFAULT_LOG_LEVEL);
        int level = std::stoi(arg_val);

        if (level <= LOG_LEVEL_WARNING) {
            BTCLOG(LOG_LEVEL_INFO) << "set log level: " << level;
            FLAGS_minloglevel = Logging::MapIntoGloglevel(level);
        }
        else {
            BTCLOG(LOG_LEVEL_INFO) << "set log level: " << level;
            FLAGS_minloglevel = 0;
            FLAGS_v = Logging::MapIntoGloglevel(level);
        }
    }
    else
        BTCLOG(LOG_LEVEL_INFO) << "default log level: " << DEFAULT_LOG_LEVEL;
    
    // chain category
    env_ = BaseEnv::mainnet;
    if (args_.IsArgSet(GLOBAL_OPTION_TESTNET)) {
        env_ = BaseEnv::testnet;
        BTCLOG(LOG_LEVEL_INFO) << "set chain category: testnet";
    }
    else if (args_.IsArgSet(GLOBAL_OPTION_REGTEST)) {
        env_ = BaseEnv::regtest;
        BTCLOG(LOG_LEVEL_INFO) << "set chain category: regtest";
    }
    
    return true;
}

/* Check options that getopt_long() can not print totally */
void ExecutorConfig::CheckOptions(int argc, const char* const argv[])
{
    if (argc == 0 || argv == nullptr)
        throw Exception(ErrorCode::invalid_argument, "argument is null");
    
    for (int i = 1; i < argc; i++) {
        std::string str(argv[i]);
        if ((str.length() > 2 && str.compare(0, 2, "--")) ||
            !str.compare("--")) {
            throw Exception(ErrorCode::invalid_option, "invalid option '" + str + "'");
        }
    }
}

bool ExecutorConfig::ParseFromFile(const fs::path& path) const
{
    std::ifstream ifs(path);
    if (!ifs.good()) {
        return true; // No config file is OK
    }
    
    std::string line;
    while (std::getline(ifs, line)) {
        line.erase(std::remove_if(line.begin(), line.end(), 
        [](unsigned char x){return std::isspace(x);}),
        line.end());
        if (line[0] == '#' || line.empty())
            continue;
        auto pos = line.find("=");
        if (pos != std::string::npos) {
            std::string str = line.substr(0, pos);
            if (!args_.IsArgSet(str) && str != GLOBAL_OPTION_CONF) {
                // Don't overwrite existing settings so command line settings override config file
                std::string str_val = line.substr(pos+1);
            }
        }
    }
    
    return true;
}

bool ExecutorConfig::LockDataDir()
{
    return true;
}

fs::path ExecutorConfig::PathHome()
{
    char *home_path = getenv("HOME");            
    if (home_path == NULL)
        return fs::path("/");
    else
        return fs::path(home_path);
}

std::string Args::GetArg(const std::string& arg, const std::string& arg_default) const
{
    LOCK(cs_args_);
    auto it = map_args_.find(arg);
    if (it != map_args_.end())
        return it->second;
    return arg_default;
}

bool Args::GetBoolArg(const std::string& arg, bool arg_default) const
{
    LOCK(cs_args_);
    auto it = map_args_.find(arg);
    if (it != map_args_.end()) {
        if (it->second.empty() || it->second == "0")
            return false;
        else
            return true;
    }
    
    return arg_default;
}

std::vector<std::string> Args::GetArgs(const std::string& arg) const
{
    LOCK(cs_args_);
    auto it = map_multi_args_.find(arg);
    if (it != map_multi_args_.end())
        return it->second;
    return {};
}

void Args::SetArg(const std::string& arg, const std::string& arg_val)
{
    LOCK(cs_args_);
    map_args_[arg] = arg_val;
    map_multi_args_[arg] = {arg_val};
}

void Args::SetArgs(const std::string& arg, const std::string& arg_val)
{
    LOCK(cs_args_);
    map_args_[arg] = arg_val;
    map_multi_args_[arg].push_back(arg_val);
}

bool Args::IsArgSet(const std::string& arg) const
{
    LOCK(cs_args_);
    return map_args_.count(arg);
}

bool Executor::BasicSetup()
{    
    // Ignore SIGPIPE, otherwise it will bring the daemon down if the client closes unexpectedly
    std::signal(SIGPIPE, SIG_IGN);
    
    std::set_new_handler(HandleAllocFail);
    
    return true;
}

void Executor::WaitForSignal()
{
    while (!sig_int_.IsReceived() && !sig_term_.IsReceived()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    BTCLOG(LOG_LEVEL_INFO) << "Caught an interrupt signal.";
}

void SetupEnvironment()
{
    try {
        std::locale(""); // Raises a runtime error if current locale is invalid
    } catch (const std::runtime_error&) {
        setenv("LC_ALL", "C", 1);
    }
}

} // namespace util
} // namespace btclite
