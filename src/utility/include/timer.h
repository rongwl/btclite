#ifndef BTCLITE_TIMER_H
#define BTCLITE_TIMER_H


#include <atomic>
#include <functional>
#include <list>
#include <mutex>
#include <thread>

#include "thread.h"
#include "utiltime.h"


struct TimerCfg {
    TimerCfg(uint32_t t, uint32_t i, uint64_t e, const std::function<void()>& c)
        : timeout(t), interval(i), expire_ms(e), suspended(false), cb(c) {}

    std::atomic<uint32_t> timeout;
    std::atomic<uint32_t> interval;
    std::atomic<uint64_t> expire_ms;
    std::atomic<bool> suspended;
    std::function<void()> cb;
};

class TimerMng : Uncopyable {
public:
    using TimerPtr = std::shared_ptr<TimerCfg>;

    TimerMng()
        : timers_(), thread_pool_(std::thread::hardware_concurrency()*2+1)
    {
        std::thread t(&TimerMng::TimerLoop, this);
        t.detach();
    }

    template <typename Func, typename... Args>
    TimerPtr NewTimer(uint32_t timeout, uint32_t interval, Func&& f, Args&&... args); 
    void ResetTimer(TimerPtr timer, uint32_t timeout = 0, uint32_t interval = 0);
    void StopTimer(TimerPtr timer);

private:
    std::mutex mutex_;
    std::list<TimerPtr> timers_;
    ThreadPool thread_pool_;

    void TimerLoop()
    {
        while (1) {
            CheckTimers();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void CheckTimers();
    void InvokeTimerCb(TimerPtr timer);
};

template <typename Func, typename... Args>
TimerMng::TimerPtr TimerMng::NewTimer(uint32_t timeout, uint32_t interval, Func&& f, Args&&... args)
{
    if (timeout == 0)
        return nullptr;

    int64_t now = GetTimeMillis();
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
