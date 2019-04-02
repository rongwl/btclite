#ifndef BTCLITE_THREAD_INTERRUPT_H
#define BTCLITE_THREAD_INTERRUPT_H

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>

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


class thread_group {
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
// mixin uncopyable
using ThreadGroup = Uncopyable<thread_group>;

template <typename F>
std::thread* thread_group::create_thread(F threadfunc)
{
	std::lock_guard<std::mutex> guard(m);
	std::unique_ptr<std::thread> new_thread(new std::thread(threadfunc));
	threads.push_back(new_thread.get());
	return new_thread.release();
}

#endif // BTCLITE_THREAD_INTERRUPT_H
