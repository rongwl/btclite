#ifndef BTCLITE_UTIL_H
#define BTCLITE_UTIL_H


#include "environment.h"
#include "fs.h"
#include "sync.h"

#include <csignal>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <getopt.h>
#include <map>
#include <set>
#include <thread>
#include <vector>


#define GLOBAL_OPTION_HELP     "help"
#define GLOBAL_OPTION_DATADIR  "datadir"
#define GLOBAL_OPTION_DEBUG    "debug"
#define GLOBAL_OPTION_CONF     "conf"
#define GLOBAL_OPTION_LOGLEVEL "loglevel"
#define GLOBAL_OPTION_TESTNET  "testnet"
#define GLOBAL_OPTION_REGTEST  "regtest"


class SigMonitor {
public:
    SigMonitor(volatile std::sig_atomic_t sig)
        : signal_(sig)
    {
        std::signal(sig, Handler);
    }
    
    bool IsReceived() const
    {
        return (received_signal_ != 0 && received_signal_ == signal_);
    }

    static volatile std::sig_atomic_t received_signal_;

private:
    volatile std::sig_atomic_t signal_;
    
    static void Handler(int sig)
    {
        received_signal_ = sig;
    }
};

// mixin class
template <class BASE>
class Uncopyable : public BASE {
public:
    using BASE::BASE;
    Uncopyable(const Uncopyable&) = delete;
    void operator=(const Uncopyable&) = delete;
};

class Args {
public:    
    virtual bool Init(int argc, const char* const argv[]) = 0;
    virtual void Parse(int argc, const char* const argv[]) = 0;
    virtual void CheckArguments() const;
    virtual bool InitParameters();
    virtual void PrintUsage() const;
    
    //-------------------------------------------------------------------------
    std::string GetArg(const std::string& arg, const std::string& arg_default) const;
    bool GetBoolArg(const std::string& arg, bool arg_default) const;
    std::vector<std::string> GetArgs(const std::string& arg) const;
    void SetArg(const std::string& arg, const std::string& arg_val);
    void SetArgs(const std::string& arg, const std::string& arg_val);
    bool IsArgSet(const std::string& arg) const;
    
    void Clear()
    {
        LOCK(cs_args_);
        map_args_.clear();
        map_multi_args_.clear();
    }
    
    bool ParseFromFile(const std::string& path) const;
    
protected:
    void CheckOptions(int argc, const char* const argv[]);
    
private:
    mutable CriticalSection cs_args_;
    std::map<std::string, std::string> map_args_;
    std::map<std::string, std::vector<std::string> > map_multi_args_;
};

class DataFiles {
public:
    DataFiles()
        : path_data_dir_(PathHome()), path_config_file_(path_data_dir_ / "btclite.conf") {}
    
    explicit DataFiles(const fs::path& data_dir)
        : path_data_dir_(data_dir), path_config_file_(path_data_dir_ / "btclite.conf") {}
    
    DataFiles(const fs::path& data_dir, const std::string& config_file)
        : path_data_dir_(data_dir), path_config_file_(path_data_dir_ / config_file) {}
    
    virtual bool Init(const std::string& path, const std::string& config_file) = 0;
    bool LockDataDir();
    static fs::path PathHome();

    //-------------------------------------------------------------------------
    const fs::path& path_data_dir() const
    {
        return path_data_dir_;
    }
    
    const fs::path& path_config_file() const
    {
        return path_config_file_;
    }
    
protected:
    void set_path_data_dir(const fs::path& path);
    void set_path_config_file(const std::string& filename);
    
private:    
    fs::path path_data_dir_;
    fs::path path_config_file_;    
};

class ExecutorConfig {
public:
    ExecutorConfig(int argc, const char* const argv[]);
    
    virtual bool Init() = 0;
    virtual const Args& args() const = 0;
    virtual const DataFiles& data_files() const = 0;
    
    //-------------------------------------------------------------------------
    int argc() const
    {
        return argc_;
    }
    
    const char* const *argv() const
    {
        return argv_;
    }
    
    BaseEnv env() const
    {
        return env_;
    }

private:
    int argc_;
    const char* const *argv_;
    BaseEnv env_;
};

class BaseExecutor {
public:    
    BaseExecutor()
        : sig_int_(SigMonitor(SIGINT)), sig_term_(SigMonitor(SIGTERM)) {}
    
    virtual bool Init() = 0;
    virtual bool Start() = 0;
    virtual bool Run() = 0;
    virtual void Interrupt() = 0;
    virtual void Stop() = 0;
    
    virtual bool BasicSetup();
    virtual void WaitForSignal();

private:
    SigMonitor sig_int_;
    SigMonitor sig_term_;
};
// mixin uncopyable
using Executor = Uncopyable<BaseExecutor>;

void SetupEnvironment();

#endif // BTCLITE_UTIL_H
