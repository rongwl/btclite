#ifndef BTCLITE_UTIL_H
#define BTCLITE_UTIL_H


#include "environment.h"
#include "fs.h"
#include "sync.h"
#include "utility/include/logging.h"

#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <getopt.h>
#include <map>
#include <set>
#include <thread>
#include <vector>
#include <iostream>

#define GLOBAL_OPTION_HELP     "help"
#define GLOBAL_OPTION_DATADIR  "datadir"
#define GLOBAL_OPTION_DEBUG    "debug"
#define GLOBAL_OPTION_CONF     "conf"
#define GLOBAL_OPTION_LOGLEVEL "loglevel"
#define GLOBAL_OPTION_TESTNET  "testnet"
#define GLOBAL_OPTION_REGTEST  "regtest"


void SetupEnvironment();

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
    virtual void ParseParameters(int argc, const char* const argv[]) = 0;
    virtual bool InitDataDir() = 0;
    virtual bool InitParameters();
    
    bool LockDataDir();
    static fs::path PathHome();
    
    BaseEnv env() const
    {
        return env_;
    }
    
    const Args& args() const
    {
        return args_;
    }
    
    const fs::path& path_data_dir() const
    {
        return path_data_dir_;
    }

protected:
    BaseEnv env_;
    Args args_;
    fs::path path_data_dir_;
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
        : sig_int_(SIGINT), sig_term_(SIGTERM) {}
    
    virtual bool Init() = 0;
    virtual bool Start() = 0;
    virtual void Interrupt() = 0;
    virtual void Stop() = 0;
    
    virtual bool BasicSetup();
    virtual void WaitForSignal();

private:
    SigMonitor sig_int_;
    SigMonitor sig_term_;
};

/* Merge from bitcoin core 0.16.3. 
 * Median filter over a stream of values.
 * Returns the median of the last N numbers
 */
template <typename T>
class MedianFilter
{
public:
    MedianFilter(unsigned int size, T initial_value)
        : size_(size)
    {
        values_.reserve(size);
        values_.push_back(initial_value);
        sorted_ = values_;
    }

    void Input(T value);
    T Median() const;

    int Size() const
    {
        return values_.size();
    }

    std::vector<T> sorted() const
    {
        return sorted_;
    }
    
private:
    std::vector<T> values_;
    std::vector<T> sorted_;
    unsigned int size_;
};

template <typename T>
void MedianFilter<T>::Input(T value)
{
    if (values_.size() == size_) {
        values_.erase(values_.begin());
    }
    values_.push_back(value);

    sorted_.resize(values_.size());
    std::copy(values_.begin(), values_.end(), sorted_.begin());
    std::sort(sorted_.begin(), sorted_.end());
}

template <typename T>
T MedianFilter<T>::Median() const
{
    int sorted_size = sorted_.size();
    assert(sorted_size > 0);
    if (sorted_size & 1) // Odd number of elements
    {
        return sorted_[sorted_size / 2];
    } else // Even number of elements
    {
        return (sorted_[sorted_size / 2 - 1] + sorted_[sorted_size / 2]) / 2;
    }
}


#endif // BTCLITE_UTIL_H
