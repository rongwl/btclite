#ifndef BTCLITE_UTIL_H
#define BTCLITE_UTIL_H


#include "btcnet.h"
#include "fs.h"
#include "sync.h"
#include "utility/include/logging.h"

#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <future>
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


namespace btclite {
namespace util {

void SetupEnvironment();


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

class Configuration : Uncopyable {
public:
    virtual void ParseParameters(int argc, const char* const argv[]) = 0;
    virtual bool InitDataDir() = 0;
    
    bool InitArgs();
    void CheckArgs() const;
    void PrintUsage(const char* const bin_name) const;
    
    bool LockDataDir();
    
    inline fs::path PathHome() const
    {
        char *home_path = getenv("HOME");
        return (home_path) ? fs::path(home_path) : fs::path("/");
    }
    
    BtcNet btcnet() const
    {
        return btcnet_;
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
    BtcNet btcnet_;
    Args args_;
    fs::path path_data_dir_;
    std::string config_file_;
    
    void CheckOptions(int argc, const char* const argv[]);
    bool ParseFromFile(const fs::path& path) const;
    
private:
    virtual bool InitArgsCustomized() = 0;
    virtual void CheckArgsCustomized() const = 0;
    virtual void PrintUsageCustomized() const = 0;
};


class Executor : Uncopyable {
public:
    virtual bool Init() = 0;
    virtual bool Start() = 0;
    virtual void Interrupt() = 0;
    virtual void Stop() = 0;
    
    bool BasicSetup();
    
    void WaitToStop()
    {
        Stopping().get_future().wait();
        BTCLOG(LOG_LEVEL_INFO) << "Caught an interrupt signal.";
    }

private:    
    virtual bool BasicSetupCustomized() = 0;
    
    static std::promise<int>& Stopping()
    {
        static std::promise<int> stopping;
        return stopping;
    }
    
    static void HandleStop(int sig)
    {
        static std::once_flag stop_mutex;
        std::call_once(stop_mutex, [&](){ Stopping().set_value(sig); });
    }
    
    static void HandleAbort(int sig);    
    static void HandleAllocFail();
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


template<typename T>
class SetOnce
{
public:
    void Set(const T& other)
    {
        std::call_once(once_flag_, [&](){ val_ = other; });
    }

    const T& Get() 
    {
        return val_; 
    }
    
private:
    T val_;
    std::once_flag once_flag_;
};

SetOnce<std::thread::id>& MainThreadId();


} // namespace util
} // namespace btclite


#endif // BTCLITE_UTIL_H
