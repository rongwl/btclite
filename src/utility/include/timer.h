#ifndef BTCLITE_TIMER_H
#define BTCLITE_TIMER_H


#include <atomic>
#include <functional>
#include <list>
#include <mutex>
#include <thread>

#include "thread.h"
#include "utiltime.h"


class TimerCfg {
public:
    TimerCfg(uint32_t t, uint32_t i, uint64_t e, const std::function<void()>& c)
        : timeout_(t), interval_(i), expire_ms_(e), suspended_(false), cb_(c) {}
    
    void Reset(uint32_t timeout = 0, uint32_t interval = 0);
    void Suspend()
    {
        set_suspended(true);
    }
    void Resume();
    
    //-------------------------------------------------------------------------
    uint32_t timeout() const
    {
        return timeout_;
    }
    
    void set_timeout(uint32_t timeout)
    {
        timeout_ = timeout;
    }
    
    uint32_t interval() const
    {
        return interval_;
    }
    
    void set_interval(uint32_t interval)
    {
        interval_ = interval;
    }
    
    uint64_t expire_ms() const
    {
        return expire_ms_;
    }
    
    void set_expire_ms(uint64_t expire_ms)
    {
        expire_ms_ = expire_ms;
    }
    
    bool suspended() const
    {
        return suspended_;
    }
    
    void set_suspended(bool suspended)
    {
        suspended_ = suspended;
    }
    
    void cb() const
    {
        cb_();
    }
    
private:
    std::atomic<uint32_t> timeout_;
    std::atomic<uint32_t> interval_;
    std::atomic<uint64_t> expire_ms_;
    std::atomic<bool> suspended_;
    std::function<void()> cb_;
};

class TimerMng : Uncopyable {
public:
    using TimerPtr = std::shared_ptr<TimerCfg>;

    TimerMng()
        : timers_(), thread_pool_(std::thread::hardware_concurrency()*2+1), stop_(false)
    {
        std::thread t(&TimerMng::TimerLoop, this);
        t.detach();
    }

    //-------------------------------------------------------------------------
    template <typename Func, typename... Args>
    TimerPtr StartTimer(uint32_t timeout, uint32_t interval, Func&& f, Args&&... args); 
    void StopTimer(TimerPtr timer);
    
    //-------------------------------------------------------------------------
    void set_stop(bool stop)
    {
        BTCLOG(LOG_LEVEL_INFO) << "Set TimerMng stop:" << stop;
        stop_ = stop;
    }

private:
    std::mutex mutex_;
    std::list<TimerPtr> timers_;
    ThreadPool thread_pool_;
    std::atomic<bool> stop_;

    void TimerLoop()
    {
        while (!stop_) {
            CheckTimers();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void CheckTimers();
    void InvokeTimerCb(TimerPtr timer);
};

template <typename Func, typename... Args>
TimerMng::TimerPtr TimerMng::StartTimer(uint32_t timeout, uint32_t interval, Func&& f, Args&&... args)
{
    if (timeout == 0)
        return nullptr;

    int64_t now = btclite::utility::util_time::GetTimeMillis();
    auto func = std::function<void()>(std::bind(std::forward<Func>(f), std::forward<Args>(args)...));
    std::lock_guard<std::mutex> lock(mutex_);
    timers_.emplace_back(std::make_shared<TimerCfg>(timeout, interval, now+timeout, func));
    return timers_.back();
}

class SingletonTimerMng : Uncopyable {
public:
    static TimerMng& GetInstance()
    {
        static TimerMng timer_mng;
        return timer_mng;
    }
    
private:
    SingletonTimerMng() {}
};


#endif // BTCLITE_TIMER_H
