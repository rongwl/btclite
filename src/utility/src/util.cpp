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


void Configuration::PrintUsage(const char* const bin_name) const
{
    fprintf(stdout, "Usage: %s [OPTIONS...]\n\n", bin_name);
    fprintf(stdout, "Common Options:\n");
    fprintf(stdout, "  -h or -?,  --help     print this help message and exit\n");
    PrintUsageCustomized();
    //              "                                                                                "

}

void Configuration::CheckArgs() const
{
    // --testnet and --regtest
    if (args_.IsArgSet(GLOBAL_OPTION_TESTNET) && args_.IsArgSet(GLOBAL_OPTION_REGTEST)) 
        throw Exception(ErrorCode::kInvalidOption, "invalid combination of --testnet and --regtest");
    
    // --debug
    if (args_.IsArgSet(GLOBAL_OPTION_DEBUG)) {
        const std::vector<std::string> arg_values = args_.GetArgs(GLOBAL_OPTION_DEBUG);
        if (std::none_of(arg_values.begin(), arg_values.end(),
        [](const std::string& val) { return val == "0"; })) {
            auto result = std::find_if(arg_values.begin(), arg_values.end(), 
            [](const std::string& module) { return logging::MapIntoModule(module) == logging::NONE; });
            if (result != arg_values.end())
                throw Exception(ErrorCode::kInvalidArg, "invalid module '" + *result + "'");
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
            throw Exception(ErrorCode::kInvalidArg, "invalid loglevel '" + arg_val + "'");
        }
        if (level < 0 || level >= LOG_LEVEL_MAX) {
            throw Exception(ErrorCode::kInvalidArg, "invalid loglevel '" + arg_val + "'");
        }
    }
    
    CheckArgsCustomized();
}

bool Configuration::InitArgs()
{    
    // --debug
    if (args_.IsArgSet(GLOBAL_OPTION_DEBUG)) {
        const std::vector<std::string> arg_values = args_.GetArgs(GLOBAL_OPTION_DEBUG);
        if (std::none_of(arg_values.begin(), arg_values.end(),
        [](const std::string& val) { return val == "0"; })) {
            for (auto module : arg_values) {
                logging::set_logModule(logging::MapIntoModule(module));
            }
        }
    }
    
    // --loglevel
    if (args_.IsArgSet(GLOBAL_OPTION_LOGLEVEL)) {
        const std::string arg_val = args_.GetArg(GLOBAL_OPTION_LOGLEVEL, DEFAULT_LOG_LEVEL);
        int level = std::stoi(arg_val);

        if (level <= LOG_LEVEL_WARNING) {
            BTCLOG(LOG_LEVEL_INFO) << "set log level: " << level;
            FLAGS_minloglevel = logging::MapIntoGloglevel(level);
        }
        else {
            BTCLOG(LOG_LEVEL_INFO) << "set log level: " << level;
            FLAGS_minloglevel = 0;
            FLAGS_v = logging::MapIntoGloglevel(level);
        }
    }
    else
        BTCLOG(LOG_LEVEL_INFO) << "default log level: " << DEFAULT_LOG_LEVEL;
    
    // bitcoin network
    btcnet_ = BtcNet::kMainNet;
    if (args_.IsArgSet(GLOBAL_OPTION_TESTNET)) {
        btcnet_ = BtcNet::kTestNet;
        BTCLOG(LOG_LEVEL_INFO) << "set bitcoin network: testnet";
    }
    else if (args_.IsArgSet(GLOBAL_OPTION_REGTEST)) {
        btcnet_ = BtcNet::kRegTest;
        BTCLOG(LOG_LEVEL_INFO) << "set bitcoin network: regtest";
    }
    
    return InitArgsCustomized();
}

BtcNet Configuration::btcnet() const
{
    return btcnet_;
}

const Args& Configuration::args() const
{
    return args_;
}

const fs::path& Configuration::path_data_dir() const
{
    return path_data_dir_;
}

/* Check options that getopt_long() can not print totally */
void Configuration::CheckOptions(int argc, const char* const argv[])
{
    if (argc == 0 || argv == nullptr)
        throw Exception(ErrorCode::kInvalidArg, "argument is null");
    
    for (int i = 1; i < argc; i++) {
        std::string str(argv[i]);
        if ((str.length() > 2 && str.compare(0, 2, "--")) ||
            !str.compare("--")) {
            throw Exception(ErrorCode::kInvalidOption, "invalid option '" + str + "'");
        }
    }
}

bool Configuration::ParseFromFile(const fs::path& path) const
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

bool Configuration::LockDataDir()
{
    return true;
}

void Args::Clear()
{
    LOCK(cs_args_);
    map_args_.clear();
    map_multi_args_.clear();
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
    std::signal(SIGINT, HandleStop); // for 'ctrl-c'
    std::signal(SIGTERM, HandleStop); // for command 'kill'
    std::signal(SIGQUIT, HandleStop); // for 'ctrl-\'    
    std::signal(SIGABRT, HandleAbort); // for assert, std::abort, std::terminate
    
    // Ignore SIGPIPE, otherwise it will bring the daemon down if the client closes unexpectedly
    std::signal(SIGPIPE, SIG_IGN);
    
    std::set_new_handler(HandleAllocFail);
    
    return BasicSetupCustomized();
}

void Executor::WaitToStop()
{
    Stopping().get_future().wait();
    BTCLOG(LOG_LEVEL_INFO) << "Caught an interrupt signal.";
}

std::promise<int>& Executor::Stopping()
{
    static std::promise<int> stopping;
    return stopping;
}

void Executor::HandleStop(int sig)
{
    static std::once_flag stop_mutex;
    std::call_once(stop_mutex, [&](){ Stopping().set_value(sig); });
}

void Executor::HandleAbort(int sig)
{
    if (std::this_thread::get_id() != MainThreadId().Get()) {
        HandleStop(sig);
        // wait for main thread to exit
        while (1) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

void Executor::HandleAllocFail()
{
    // Rather than throwing std::bad-alloc if allocation fails, terminate
    // immediately to (try to) avoid chain corruption.
    std::set_new_handler(nullptr);
    
    BTCLOG(LOG_LEVEL_ERROR) << "Critical error: out of memory. Terminating.";
    
    std::signal(SIGABRT, [](int){});
    // The log was successful, terminate now.
    std::terminate();
}

void SetupEnvironment()
{
    try {
        std::locale(""); // Raises a runtime error if current locale is invalid
    } catch (const std::runtime_error&) {
        setenv("LC_ALL", "C", 1);
    }
}

SetOnce<std::thread::id>& MainThreadId()
{
    static auto *p_main_thread_id = new SetOnce<std::thread::id>();
    return *p_main_thread_id;
}

} // namespace util
} // namespace btclite
