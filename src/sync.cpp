#include "sync.h"
#include "util.h"
#include "utilstrencodings.h"

#include <sstream>
#include <boost/thread.hpp>

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
		: mutex_name(psz_name), file(psz_file), line(l), is_try(try_lock) {}
	
	std::string ToString() const
	{
		std::stringstream ss;
		ss << mutex_name << " " << file << ":" << line << (is_try ? " (TRY)" : "");
		return ss.str();
	}
	std::string MutexName() const
	{
		return mutex_name;
	}
	bool isTry() const
	{
		return is_try;
	}
private:
	std::string mutex_name;
	std::string file;
	int line;
	bool is_try;
};

typedef std::vector<std::pair<void*, CLockLocation> > LockStack;
typedef std::map<std::pair<void*, void*>, LockStack> LockOrders;
typedef std::set<std::pair<void*, void*> > InvLockOrders;

boost::thread_specific_ptr<LockStack> lock_stack;

struct LockData {
	bool available;
	LockData()
		: available(true) {}
	~LockData()
	{
		available = false;
	}

	LockOrders lock_orders;
	InvLockOrders invlock_orders;
	std::mutex mutex;
} static lock_data;

static void potential_deadlock_detected(const std::pair<void*, void*>& mismatch, const LockStack& s1, const LockStack& s2)
{
	std::cout << "POTENTIAL DEADLOCK DETECTED" << std::endl;
	std::cout << "Previous lock order was:" << std::endl;
	for (auto it : s1) {
		if (it.first == mismatch.first)
			std::cout << " (1)";
		else if (it.first == mismatch.second)
			std::cout << " (2)";
		std::cout << it.second.ToString() << std::endl;
	}
	std::cout << "Current lock order is:" << std::endl;
	for (auto it : s2) {
		if (it.first == mismatch.first)
			std::cout << " (1)";
		else if (it.first == mismatch.second)
			std::cout << " (2)";
		std::cout << it.second.ToString() << std::endl;
	}
	
	assert(false);
}

void PushLock(void *c, const CLockLocation& lock_location, bool try_lock)
{
	if (lock_stack.get() == NULL)
		lock_stack.reset(new LockStack);

	std::unique_lock<std::mutex> lock(lock_data.mutex);
	(*lock_stack).push_back(std::make_pair(c, lock_location));
	for (auto it : *lock_stack) {
		if (it.first == c)
			break;
		std::pair<void*, void*> pair = std::make_pair(it.first, c);
		lock_data.lock_orders[pair] = *lock_stack;
		std::pair<void*, void*> invpair = std::make_pair(c, it.first);
		lock_data.invlock_orders.insert(invpair);
		if (lock_data.lock_orders.count(invpair))
			potential_deadlock_detected(pair, lock_data.lock_orders[invpair], lock_data.lock_orders[pair]);
	}
}

void PopLock()
{
	(*lock_stack).pop_back();
}

void DeleteLock(void *cs)
{
	if(!lock_data.available)
		return;

	std::unique_lock<std::mutex> lock(lock_data.mutex);
	auto it = lock_data.lock_orders.lower_bound(std::make_pair(cs, (void*)0));
	while (it != lock_data.lock_orders.end() && it->first.first == cs) {
		lock_data.lock_orders.erase(it);
		lock_data.invlock_orders.erase(std::make_pair(it->first.second, it->first.first));
		it++;
	}
	auto invit = lock_data.invlock_orders.lower_bound(std::make_pair(cs, (void *)0));
	while (invit != lock_data.invlock_orders.end() && invit->first == cs) {
		lock_data.invlock_orders.erase(invit);
		lock_data.lock_orders.erase(std::make_pair(invit->second, invit->first));
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
