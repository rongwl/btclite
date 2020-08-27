#ifndef BTCLITE_TIMER_H
#define BTCLITE_TIMER_H


#include <atomic>
#include <functional>
#include <list>
#include <mutex>
#include <thread>

#include "thread.h"
#include "util_time.h"


namespace btclite {
namespace util {

class TimerCfg {
public:
    TimerCfg(uint32_t t, uint32_t i, uint64_t e, 
             const std::function<void()>& c);
    
    //-------------------------------------------------------------------------
    void Reset(uint32_t timeout = 0, uint32_t interval = 0);
    void Suspend();
    void Resume();
    
    //-------------------------------------------------------------------------
    uint32_t timeout() const;   
    void set_timeout(uint32_t timeout);   
    uint32_t interval() const;   
    void set_interval(uint32_t interval);   
    uint64_t expire_ms() const;    
    void set_expire_ms(uint64_t expire_ms);   
    bool suspended() const;   
    void set_suspended(bool suspended);    
    void cb() const;
    
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

    TimerMng();

    //-------------------------------------------------------------------------
    template <typename Func, typename... Args>
    TimerPtr StartTimer(uint32_t timeout, uint32_t interval, Func&& f, Args&&... args); 
    void StopTimer(TimerPtr timer);
    
    //-------------------------------------------------------------------------
    void set_stop(bool stop);

private:
    std::mutex mutex_;
    std::list<TimerPtr> timers_;
    ThreadPool thread_pool_;
    std::atomic<bool> stop_;

    void TimerLoop();
    void CheckTimers();
    void InvokeTimerCb(TimerPtr timer);
};

template <typename Func, typename... Args>
TimerMng::TimerPtr TimerMng::StartTimer(uint32_t timeout, uint32_t interval, Func&& f, Args&&... args)
{
    if (timeout == 0)
        return nullptr;

    int64_t now = GetTimeMillis();
    auto func = std::bind(std::forward<Func>(f), std::forward<Args>(args)...);
    std::lock_guard<std::mutex> lock(mutex_);
    timers_.emplace_back(std::make_shared<TimerCfg>(timeout, interval, now+timeout, func));
    return timers_.back();
}

class SingletonTimerMng : Uncopyable {
public:
    static TimerMng& GetInstance();
    
private:
    SingletonTimerMng() {}
};

} // namespace util
} // namespace btclite

#endif // BTCLITE_TIMER_H
