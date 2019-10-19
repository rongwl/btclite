#include "sync.h"

#include <set>
#include <sstream>

#include "utility/include/logging.h"


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

class LockLocation {
public:
    LockLocation(const char *psz_name, const char *psz_file, int l, bool try_lock)
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

using LockStack = std::vector<std::pair<void*, LockLocation> >;
using LockOrders = std::map<std::pair<void*, void*>, LockStack>;
using InvLockOrders = std::set<std::pair<void*, void*> >;

static thread_local LockStack g_lock_stack;

namespace {
struct LockData {
    // Very ugly hack: as the global constructs and destructors run single
    // threaded, we use this boolean to know whether LockData still exists,
    // as DeleteLock can get called by global CriticalSection destructors
    // after LockData disappears.
    bool available;
    LockData() : available(true) {}
    ~LockData() { available = false; }

    LockOrders lock_orders_;
    InvLockOrders invlock_orders_;
    std::mutex lock_stack_mutex_;
} lock_data;
}

static void potential_deadlock_detected(const std::pair<void*, void*>& mismatch, const LockStack& s1, const LockStack& s2)
{
    BTCLOG(LOG_LEVEL_ERROR) << "POTENTIAL DEADLOCK DETECTED";
    BTCLOG(LOG_LEVEL_ERROR) << "Previous lock order was:";
    for (auto it : s1) {
        if (it.first == mismatch.first)
            BTCLOG(LOG_LEVEL_ERROR) << " (1)";
        else if (it.first == mismatch.second)
            BTCLOG(LOG_LEVEL_ERROR) << " (2)";
        BTCLOG(LOG_LEVEL_ERROR) << " " << it.second.ToString();
    }
    BTCLOG(LOG_LEVEL_ERROR) << "Current lock order is:";
    for (auto it : s2) {
        if (it.first == mismatch.first)
            BTCLOG(LOG_LEVEL_ERROR) << " (1)";
        else if (it.first == mismatch.second)
            BTCLOG(LOG_LEVEL_ERROR) << " (2)";
        BTCLOG(LOG_LEVEL_ERROR) << " " << it.second.ToString();
    }
    
    assert(false);
}

void PushLock(void *c, const LockLocation& lock_location, bool try_lock)
{
    std::unique_lock<std::mutex> lock(lock_data.lock_stack_mutex_);
    g_lock_stack.push_back(std::make_pair(c, lock_location));
    for (auto it : g_lock_stack) {
        if (it.first == c)
            break;
        std::pair<void*, void*> pair = std::make_pair(it.first, c);
        lock_data.lock_orders_[pair] = g_lock_stack;
        std::pair<void*, void*> invpair = std::make_pair(c, it.first);
        lock_data.invlock_orders_.insert(invpair);
        if (lock_data.lock_orders_.count(invpair))
            potential_deadlock_detected(pair, lock_data.lock_orders_[invpair], lock_data.lock_orders_[pair]);
    }
}

void PopLock()
{
    g_lock_stack.pop_back();
}

void DeleteLock(void *cs)
{
    if (!lock_data.available)  // We're already shutting down.
        return;
    
    std::unique_lock<std::mutex> lock(lock_data.lock_stack_mutex_);
    auto it = lock_data.lock_orders_.lower_bound(std::make_pair(cs, (void*)0));
    while (it != lock_data.lock_orders_.end() && it->first.first == cs) {
        lock_data.lock_orders_.erase(it);
        lock_data.invlock_orders_.erase(std::make_pair(it->first.second, it->first.first));
        ++it;
    }
    auto invit = lock_data.invlock_orders_.lower_bound(std::make_pair(cs, (void *)0));
    while (invit != lock_data.invlock_orders_.end() && invit->first == cs) {
        lock_data.invlock_orders_.erase(invit);
        lock_data.lock_orders_.erase(std::make_pair(invit->second, invit->first));
        ++invit;
    }
}

void EnterCritical(const char *psz_name, const char *psz_file, int line, void *cs, bool try_lock)
{
    PushLock(cs, LockLocation(psz_name, psz_file, line, try_lock), try_lock);
}

void LeaveCritical()
{
    PopLock();
}

#endif // CHECK_LOCKORDER
