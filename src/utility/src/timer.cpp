#include "timer.h"


void TimerMng::ResetTimer(TimerPtr timer, uint32_t timeout, uint32_t interval)
{
    int64_t now = Time::GetTimeMillis();
    if (timeout == 0)
        timer->expire_ms = now + timer->timeout;
    else {
        timer->expire_ms = now + timeout;
        timer->interval = interval;
    }
}

void TimerMng::StopTimer(TimerPtr timer)
{
    std::lock_guard<std::mutex> lock(mutex_);
    timers_.erase(std::find(timers_.begin(), timers_.end(), timer));
}

void TimerMng::CheckTimers()
{
    std::lock_guard<std::mutex> lock(mutex_);
    uint64_t now = Time::GetTimeMillis();
    for (auto it = timers_.begin(); it != timers_.end(); ++it) {
        if ((*it)->suspended == false && now >= (*it)->expire_ms) {
            (*it)->suspended = true;
            auto task = std::bind(&TimerMng::InvokeTimerCb, this, std::placeholders::_1);
            thread_pool_.AddTask(std::function<void(TimerPtr)>(task), *it);
        }
    }
}

void TimerMng::InvokeTimerCb(TimerPtr timer)
{
    timer->cb();
    if (timer->interval > 0) {
        timer->expire_ms = Time::GetTimeMillis() + timer->interval;
    }
    else {
        std::lock_guard<std::mutex> lock(mutex_);
        timers_.erase(std::find(timers_.begin(), timers_.end(), timer));
    }
    timer->suspended = false;
}
