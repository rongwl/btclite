#include "sync.h"
#include "util.h"
#include "utilstrencodings.h"

#include <sstream>

#ifdef CHECK_LOCKORDER
//
// Early deadlock detection.
// Problem being solved:
//    Thread 1 locks  A, then B, then C
//    Thread 2 locks  D, then C, then A
//     --> may result in deadlock between the two threads, depending on when they run.
// Solution implemented here:
// Keep track of pairs of locks: (A before B), (A before C), etc.
// Complain if any thread tries to lock in a different order.
//

class CLockLocation {
public:
	CLockLocation(const char *psz_name, const char *psz_file, int l, bool try_lock)
		: mutex_name_(psz_name), file_(psz_file), line_(l), is_try_(try_lock) {}
	
	std::string ToString() const
	{
		std::stringstream ss;
		ss << mutex_name_ << " " << file_ << ":" << line_ << (is_try_ ? " (TRY)" : "");
		return ss.str();
	}
	std::string MutexName() const
	{
		return mutex_name_;
	}
	bool IsTry() const
	{
		return is_try_;
	}
private:
	std::string mutex_name_;
	std::string file_;
	int line_;
	bool is_try_;
};

typedef std::vector<std::pair<void*, CLockLocation> > LockStack;
typedef std::map<std::pair<void*, void*>, LockStack> LockOrders;
typedef std::set<std::pair<void*, void*> > InvLockOrders;

static thread_local LockStack g_lock_stack;

namespace {
	LockOrders lock_orders;
	InvLockOrders invlock_orders;
	std::mutex lock_stack_mutex;
}

static void potential_deadlock_detected(const std::pair<void*, void*>& mismatch, const LockStack& s1, const LockStack& s2)
{
	LogPrintf("POTENTIAL DEADLOCK DETECTED\n");
    LogPrintf("Previous lock order was:\n");
	for (auto it : s1) {
		if (it.first == mismatch.first)
			LogPrintf(" (1)");
		else if (it.first == mismatch.second)
			LogPrintf(" (2)");
		LogPrintf(" %s\n", it.second.ToString());
	}
	LogPrintf("Current lock order is:\n");
	for (auto it : s2) {
		if (it.first == mismatch.first)
			LogPrintf(" (1)");
		else if (it.first == mismatch.second)
			LogPrintf(" (2)");
		LogPrintf(" %s\n", it.second.ToString());
	}
	
	assert(false);
}

void PushLock(void *c, const CLockLocation& lock_location, bool try_lock)
{
	std::unique_lock<std::mutex> lock(lock_stack_mutex);
	g_lock_stack.push_back(std::make_pair(c, lock_location));
	for (auto it : g_lock_stack) {
		if (it.first == c)
			break;
		std::pair<void*, void*> pair = std::make_pair(it.first, c);
		lock_orders[pair] = g_lock_stack;
		std::pair<void*, void*> invpair = std::make_pair(c, it.first);
		invlock_orders.insert(invpair);
		if (lock_orders.count(invpair))
			potential_deadlock_detected(pair, lock_orders[invpair], lock_orders[pair]);
	}
}

void PopLock()
{
	g_lock_stack.pop_back();
}

void DeleteLock(void *cs)
{
	std::unique_lock<std::mutex> lock(lock_stack_mutex);
	auto it = lock_orders.lower_bound(std::make_pair(cs, (void*)0));
	while (it != lock_orders.end() && it->first.first == cs) {
		lock_orders.erase(it);
		invlock_orders.erase(std::make_pair(it->first.second, it->first.first));
		it++;
	}
	auto invit = invlock_orders.lower_bound(std::make_pair(cs, (void *)0));
	while (invit != invlock_orders.end() && invit->first == cs) {
		invlock_orders.erase(invit);
		lock_orders.erase(std::make_pair(invit->second, invit->first));
		invit++;
	}
}

void EnterCritical(const char *psz_name, const char *psz_file, int line, void *cs, bool try_lock)
{
	PushLock(cs, CLockLocation(psz_name, psz_file, line, try_lock), try_lock);
}

void LeaveCritical()
{
	PopLock();
}

#endif // CHECK_LOCKORDER
