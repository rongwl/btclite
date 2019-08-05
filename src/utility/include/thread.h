#ifndef BTCLITE_THREAD_INTERRUPT_H
#define BTCLITE_THREAD_INTERRUPT_H

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <list>
#include <memory>
#include <mutex>
#include <queue>

#include "util.h"


/*
    A helper class for interruptible sleeps. Calling operator() will interrupt
    any current sleep, and after that point operator bool() will return true
    until reset.
*/
class ThreadInterrupt
{
public:
    explicit operator bool() const;
    void operator()();
    void reset();
    bool sleep_for(std::chrono::milliseconds rel_time);
    bool sleep_for(std::chrono::seconds rel_time);
    bool sleep_for(std::chrono::minutes rel_time);

private:
    std::condition_variable cond;
    std::mutex mut;
    std::atomic<bool> flag;
};


class ThreadGroup : Uncopyable {
public:
    ThreadGroup() {}
    ~ThreadGroup()
    {
        for (auto it = threads_.begin(), end = threads_.end(); it != end; ++it)
            delete *it;
    }

    //-------------------------------------------------------------------------
    bool IsThisThreadIn();
    bool IsThreadIn(std::thread* thrd);

    //-------------------------------------------------------------------------
    template <typename Func, typename... Args>
    std::thread* CreateThread(Func&& threadfunc, Args&&... args);
    void AddThread(std::thread *thrd);
    void RemoveThread(std::thread *thrd);
    void JoinAll();

    //-------------------------------------------------------------------------
    ssize_t Size() const
    {
        std::lock_guard<std::mutex> guard(mutex_);
        return threads_.size();
    }

private:
    mutable std::mutex mutex_;
    std::list<std::thread *> threads_;
};


template <typename Func, typename... Args>
std::thread* ThreadGroup::CreateThread(Func&& threadfunc, Args&&... args)
{
    std::lock_guard<std::mutex> guard(mutex_);
    std::unique_ptr<std::thread> new_thread(new std::thread(threadfunc, args...));
    threads_.push_back(new_thread.get());
    return new_thread.release();
}

class ThreadPool : Uncopyable {
public:
    ThreadPool(size_t);
    ~ThreadPool();
    
    template<typename Func, typename... Args>
    std::future<typename std::result_of<Func(Args...)>::type> AddTask(Func&& f, Args&&... args);
    
private:
    // need to keep track of threads so we can join them
    std::vector<std::thread> threads_;
    // the task queue
    std::queue<std::function<void()> > tasks_;
    
    // synchronization
    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;
};

template<class Func, class... Args>
std::future<typename std::result_of<Func(Args...)>::type> ThreadPool::AddTask(Func&& f, Args&&... args) 
{
    using return_type = typename std::result_of<Func(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()> >(
                    std::bind(std::forward<Func>(f), std::forward<Args>(args)...)
                );
    
    std::future<return_type> ret = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);

        // don't allow enqueueing after stopping the pool
        if(stop_)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks_.emplace([task](){ (*task)(); });
    }
    condition_.notify_one();
    
    return ret;
}

class SingletonThreadPool : Uncopyable {
public:
    static ThreadPool& GetInstance()
    {
        static ThreadPool thread_pool(2 * std::thread::hardware_concurrency() + 1);
        return thread_pool;
    }
    
private:
    SingletonThreadPool() {}
};

template <typename Func>
void TraceThread(const std::string& name,  Func&& func)
{
    try
    {
        BTCLOG(LOG_LEVEL_INFO) << name << " thread start";
        func();
        BTCLOG(LOG_LEVEL_INFO) << name << " thread exit";
    }
    catch (const std::exception& e) {
        BTCLOG(LOG_LEVEL_WARNING) << name << " thread exception: " << e.what();
        throw;
    }
    catch (...) {
        BTCLOG(LOG_LEVEL_WARNING) << name << " thread unknown exception";
        throw;
    }
}

#endif // BTCLITE_THREAD_INTERRUPT_H
