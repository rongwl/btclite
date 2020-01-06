#include "timer.h"


namespace btclite {
namespace util {

void TimerCfg::Reset(uint32_t timeout, uint32_t interval)
{
   int64_t now = GetTimeMillis();
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
        expire_ms_ = GetTimeMillis() + interval_;
    set_suspended(false);
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
    uint64_t now = GetTimeMillis();
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
        timer->set_expire_ms(GetTimeMillis() + timer->interval());
    }
    else {
        std::lock_guard<std::mutex> lock(mutex_);
        timers_.erase(std::find(timers_.begin(), timers_.end(), timer));
    }
}

} // namespace util
} // namespace btclite
