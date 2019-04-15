
#include "thread.h"

ThreadInterrupt::operator bool() const
{
    return flag.load(std::memory_order_acquire);
}

void ThreadInterrupt::reset()
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

bool ThreadInterrupt::sleep_for(std::chrono::milliseconds rel_time)
{
    std::unique_lock<std::mutex> lock(mut);
    return !cond.wait_for(lock, rel_time, [this]() { return flag.load(std::memory_order_acquire); });
}

bool ThreadInterrupt::sleep_for(std::chrono::seconds rel_time)
{
    return sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(rel_time));
}

bool ThreadInterrupt::sleep_for(std::chrono::minutes rel_time)
{
    return sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(rel_time));
}


bool thread_group::is_this_thread_in()
{
    std::thread::id id = std::this_thread::get_id();
    std::lock_guard<std::mutex> guard(m);
    for (auto it = threads.begin(), end = threads.end(); it != end; ++it) {
        if ((*it)->get_id() == id)
            return true;
    }
    return false;
}

bool thread_group::is_thread_in(std::thread* thrd)
{
    if (!thrd)
        return false;

    std::thread::id id = thrd->get_id();
    std::lock_guard<std::mutex> guard(m);
    for (auto it = threads.begin(), end = threads.end(); it != end; ++it) {
        if ((*it)->get_id() == id)
            return true;
    }
    return false;
}

void thread_group::add_thread(std::thread *thrd)
{
    if (!thrd)
        return;
    if (is_thread_in(thrd))
        throw std::runtime_error("thread_group: trying to add a duplicated thread");
    std::lock_guard<std::mutex> guard(m);
    threads.push_back(thrd);
}

void thread_group::remove_thread(std::thread *thrd)
{
    std::lock_guard<std::mutex> guard(m);
    auto it = std::find(threads.begin(), threads.end(), thrd);
    if (it != threads.end())
        threads.erase(it);
}

void thread_group::join_all()
{
    if (is_this_thread_in())
        throw std::runtime_error("thread_group: trying joining itself");
    std::lock_guard<std::mutex> guard(m);
    for (auto it = threads.begin(); it != threads.end(); ++it) {
        if ((*it)->joinable())
            (*it)->join();
    }
}
