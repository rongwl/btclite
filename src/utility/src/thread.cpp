#include "thread.h"


void SetThreadName(const char* name)
{

}

ThreadInterrupt::operator bool() const
{
    return flag.load(std::memory_order_acquire);
}

void ThreadInterrupt::Reset()
{
    flag.store(false, std::memory_order_release);
}

void ThreadInterrupt::operator()()
{
    {
        std::unique_lock<std::mutex> lock(mut);
        flag.store(true, std::memory_order_release);
    }
    cond.notify_all();
}

bool ThreadInterrupt::Sleep_for(std::chrono::milliseconds rel_time)
{
    std::unique_lock<std::mutex> lock(mut);
    return !cond.wait_for(lock, rel_time, [this]() { return flag.load(std::memory_order_acquire); });
}

bool ThreadInterrupt::Sleep_for(std::chrono::seconds rel_time)
{
    return Sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(rel_time));
}

bool ThreadInterrupt::Sleep_for(std::chrono::minutes rel_time)
{
    return Sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(rel_time));
}


bool ThreadGroup::IsThisThreadIn()
{
    std::thread::id id = std::this_thread::get_id();
    std::lock_guard<std::mutex> guard(mutex_);
    for (auto it = threads_.begin(), end = threads_.end(); it != end; ++it) {
        if ((*it)->get_id() == id)
            return true;
    }
    return false;
}

bool ThreadGroup::IsThreadIn(std::thread* thrd)
{
    if (!thrd)
        return false;

    std::thread::id id = thrd->get_id();
    std::lock_guard<std::mutex> guard(mutex_);
    for (auto it = threads_.begin(), end = threads_.end(); it != end; ++it) {
        if ((*it)->get_id() == id)
            return true;
    }
    return false;
}

void ThreadGroup::AddThread(std::thread *thrd)
{
    if (!thrd)
        return;
    if (IsThreadIn(thrd))
        throw std::runtime_error("ThreadGroup: trying to add a duplicated thread");
    std::lock_guard<std::mutex> guard(mutex_);
    threads_.push_back(thrd);
}

void ThreadGroup::RemoveThread(std::thread *thrd)
{
    std::lock_guard<std::mutex> guard(mutex_);
    auto it = std::find(threads_.begin(), threads_.end(), thrd);
    if (it != threads_.end())
        threads_.erase(it);
}

void ThreadGroup::JoinAll()
{
    if (IsThisThreadIn())
        throw std::runtime_error("ThreadGroup: trying joining itself");
    std::lock_guard<std::mutex> guard(mutex_);
    for (auto it = threads_.begin(); it != threads_.end(); ++it) {
        if ((*it)->joinable())
            (*it)->join();
    }
}

ThreadPool::ThreadPool(size_t threads)
    : stop_(false)
{
    for(size_t i = 0; i < threads; ++i)
        threads_.emplace_back(
            [this]
    {
        while(1)
        {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(this->queue_mutex_);
                this->condition_.wait(lock,
                [this]{ return this->stop_ || !this->tasks_.empty(); });
                if(this->stop_ && this->tasks_.empty())
                    return;
                task = std::move(this->tasks_.front());
                this->tasks_.pop();
            }

            task();
        }
    }
    );
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    condition_.notify_all();
    for(std::thread &worker: threads_)
        worker.join();
}
