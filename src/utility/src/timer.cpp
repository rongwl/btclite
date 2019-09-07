#include "timer.h"


void TimerCfg::Reset(uint32_t timeout, uint32_t interval)
{
   int64_t now = Time::GetTimeMillis();
   if (timeout == 0) {
        expire_ms_ = now + timeout_;
        if (interval != 0)
            interval_ = interval;
   }
   else {
       expire_ms_ = now + timeout;
       if (interval != 0)
           interval_ = interval;
   }
}

void TimerCfg::Resume()
{
    if (interval_ > 0)
        expire_ms_ = Time::GetTimeMillis() + interval_;
    suspended_ = false;
}

void TimerMng::StopTimer(TimerPtr timer)
{
    if (!timer)
        return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    timers_.erase(std::find(timers_.begin(), timers_.end(), timer));
}

void TimerMng::CheckTimers()
{
    std::lock_guard<std::mutex> lock(mutex_);
    uint64_t now = Time::GetTimeMillis();
    for (auto it = timers_.begin(); it != timers_.end(); ++it) {
        if ((*it)->suspended() == false && now >= (*it)->expire_ms()) {
            (*it)->set_expire_ms(std::numeric_limits<uint64_t>::max());
            auto task = std::bind(&TimerMng::InvokeTimerCb, this, std::placeholders::_1);
            thread_pool_.AddTask(std::function<void(TimerPtr)>(task), *it);
        }
    }
}

void TimerMng::InvokeTimerCb(TimerPtr timer)
{
    if (!timer)
        return;
    
    timer->cb();
    if (timer->interval() > 0) {
        timer->set_expire_ms(Time::GetTimeMillis() + timer->interval());
    }
    else {
        std::lock_guard<std::mutex> lock(mutex_);
        timers_.erase(std::find(timers_.begin(), timers_.end(), timer));
    }
}
