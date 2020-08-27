#include "timer.h"


namespace btclite {
namespace util {

TimerCfg::TimerCfg(uint32_t t, uint32_t i, uint64_t e, 
                   const std::function<void()>& c)
    : timeout_(t), interval_(i), expire_ms_(e), suspended_(false), 
      cb_(c) 
{
}

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

void TimerCfg::Suspend()
{
    set_suspended(true);
}

void TimerCfg::Resume()
{
    if (interval_ > 0)
        expire_ms_ = GetTimeMillis() + interval_;
    set_suspended(false);
}

uint32_t TimerCfg::timeout() const
{
    return timeout_;
}

void TimerCfg::set_timeout(uint32_t timeout)
{
    timeout_ = timeout;
}

uint32_t TimerCfg::interval() const
{
    return interval_;
}

void TimerCfg::set_interval(uint32_t interval)
{
    interval_ = interval;
}

uint64_t TimerCfg::expire_ms() const
{
    return expire_ms_;
}

void TimerCfg::set_expire_ms(uint64_t expire_ms)
{
    expire_ms_ = expire_ms;
}

bool TimerCfg::suspended() const
{
    return suspended_;
}

void TimerCfg::set_suspended(bool suspended)
{
    suspended_ = suspended;
}

void TimerCfg::cb() const
{
    cb_();
}

TimerMng::TimerMng()
    : timers_(), thread_pool_(std::thread::hardware_concurrency()*2+1), 
      stop_(false)
{
    std::thread t(&TimerMng::TimerLoop, this);
    t.detach();
}

void TimerMng::StopTimer(TimerPtr timer)
{
    if (!timer)
        return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    timers_.erase(std::find(timers_.begin(), timers_.end(), timer));
}

void TimerMng::set_stop(bool stop)
{
    BTCLOG(LOG_LEVEL_INFO) << "Set TimerMng stop:" << stop;
    stop_ = stop;
}

void TimerMng::TimerLoop()
{
    while (!stop_) {
        CheckTimers();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
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

TimerMng& SingletonTimerMng::GetInstance()
{
    static TimerMng timer_mng;
    return timer_mng;
}

} // namespace util
} // namespace btclite
