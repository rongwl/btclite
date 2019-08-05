#ifndef BTCLITE_UTIL_H
#define BTCLITE_UTIL_H


#include "environment.h"
#include "fs.h"
#include "sync.h"
#include "utility/include/logging.h"

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

class Uncopyable {
public:
    Uncopyable(const Uncopyable&) = delete;
    Uncopyable& operator=(const Uncopyable&) = delete;
    
protected:
    Uncopyable() {}
};

class Args : Uncopyable {
public:
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

private:
    mutable CriticalSection cs_args_;
    std::map<std::string, std::string> map_args_;
    std::map<std::string, std::vector<std::string> > map_multi_args_;
};

class ExecutorConfig : Uncopyable {
public:
    virtual void Parse(int argc, const char* const argv[]) = 0;
    virtual bool InitDataDir() = 0;
    virtual bool InitParameters();
    
    bool LockDataDir();
    static fs::path PathHome();
    
    static const Args& args()
    {
        return args_;
    }
    
    static const fs::path& path_data_dir()
    {
        return path_data_dir_;
    }

protected:
    static Args args_;
    static fs::path path_data_dir_;
    std::string config_file_;
    
    virtual void CheckArguments() const;
    void CheckOptions(int argc, const char* const argv[]);
    bool ParseFromFile(const fs::path& path) const;
};

class HelpInfo {
public:
    static void PrintUsage();
};

class Executor : Uncopyable {
public:    
    Executor()
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


void SetupEnvironment();

#endif // BTCLITE_UTIL_H
