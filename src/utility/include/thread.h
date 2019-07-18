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

#include "utility/include/logging.h"
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


class thread_group : Uncopyable {
public:
    thread_group() {}
    ~thread_group()
    {
        for (auto it = threads.begin(), end = threads.end(); it != end; ++it)
            delete *it;
    }

    //-------------------------------------------------------------------------
    bool is_this_thread_in();
    bool is_thread_in(std::thread* thrd);

    //-------------------------------------------------------------------------
    template <typename F>
    std::thread* create_thread(F threadfunc);
    void add_thread(std::thread *thrd);
    void remove_thread(std::thread *thrd);
    void join_all();

    //-------------------------------------------------------------------------
    ssize_t size() const
    {
        std::lock_guard<std::mutex> guard(m);
        return threads.size();
    }

private:
    mutable std::mutex m;
    std::list<std::thread *> threads;
};


template <typename F>
std::thread* thread_group::create_thread(F threadfunc)
{
    std::lock_guard<std::mutex> guard(m);
    std::unique_ptr<std::thread> new_thread(new std::thread(threadfunc));
    threads.push_back(new_thread.get());
    return new_thread.release();
}

class ThreadPool {
public:
    ThreadPool(size_t);
    template<class F, class... Args>
    std::future<typename std::result_of<F(Args...)>::type> enqueue(F&& f, Args&&... args); 
    ~ThreadPool();
    
private:
    // need to keep track of threads so we can join them
    std::vector< std::thread > workers_;
    // the task queue
    std::queue< std::function<void()> > tasks_;
    
    // synchronization
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;
};

// add new work item to the pool
template<class F, class... Args>
std::future<typename std::result_of<F(Args...)>::type> ThreadPool::enqueue(F&& f, Args&&... args) 
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
                    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
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


template <typename Callable>
void TraceThread(const std::string& name,  Callable func)
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
