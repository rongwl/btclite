#ifndef BTCLITE_SYNC_H
#define BTCLITE_SYNC_H

#include <condition_variable>
#include <mutex>
#include <thread>

#ifdef CHECK_LOCKORDER
void EnterCritical(const char*, const char*, int, void*, bool try_lock = false);
void LeaveCritical();
void DeleteLock(void*);
#else
void static inline EnterCritical(const char*, const char*, int, void*, bool try_lock = false) {}
void static inline LeaveCritical() {}
void static inline DeleteLock(void*) {}
#endif

template <typename Mutex>
class MutexLock {
public:
    MutexLock(Mutex& mutex, const char *psz_name, const char *psz_file, int line, bool try_lock = false)
        : lock(mutex, std::defer_lock)
    {
        if (try_lock) 
            TryEnter(psz_name, psz_file, line);
        else
            Enter(psz_name, psz_file, line);
    }

    MutexLock(Mutex *pmutex, const char *psz_name, const char *psz_file, int line, bool try_lock = false)
    {
        if (!pmutex)
            return;
        lock = std::unique_lock<Mutex>(*pmutex, std::defer_lock);
        if (try_lock)
            TryEnter(psz_name, psz_file, line);
        else
            Enter(psz_name, psz_file, line);
    }

    ~MutexLock() 
    {
        if (lock.owns_lock())
            LeaveCritical();
    }

    operator bool()
    {
        return lock.owns_lock();
    }
private:
    std::unique_lock<Mutex> lock;

    void Enter(const char *psz_name, const char *psz_file, int line)
    {
        EnterCritical(psz_name, psz_file, line, (void*)lock.mutex());
        lock.lock();
    }
    bool TryEnter(const char *psz_name, const char *psz_file, int line)
    {
        EnterCritical(psz_name, psz_file, line, (void*)lock.mutex(), true);
        lock.try_lock();
        if (!lock.owns_lock())
            LeaveCritical();
        return lock.owns_lock();
    }
};

class CriticalSection : public std::recursive_mutex {
public:
    ~CriticalSection()
    {
        DeleteLock((void*)this);
    }
};

using CriticalBlock = MutexLock<CriticalSection>;

#define PASTE(x, y) x ## y
#define PASTE2(x, y) PASTE(x, y)

#define LOCK(cs) CriticalBlock PASTE2(criticalblock, __COUNTER__)(cs, #cs, __FILE__, __LINE__)
#define TRY_LOCK(cs, name) CriticalBlock name(cs, #cs, __FILE__, __LINE__, true)


class Semaphore
{
public:
    explicit Semaphore(int init) : value(init) {}

    void wait()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [&]() { return value >= 1; });
        value--;
    }

    bool try_wait()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (value < 1)
            return false;
        value--;
        return true;
    }

    void post()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            value++;
        }
        condition_.notify_one();
    }
    
private:
    std::condition_variable condition_;
    std::mutex mutex_;
    int value;
};

/** RAII-style semaphore lock */
class SemaphoreGrant
{
public:
    SemaphoreGrant()
        : sem_(nullptr), have_grant_(false) {}

    explicit SemaphoreGrant(Semaphore& sem, bool fTry = false)
        : sem_(&sem), have_grant_(false)
    {
        if (fTry)
            TryAcquire();
        else
            Acquire();
    }

    ~SemaphoreGrant()
    {
        Release();
    }
    
    void Acquire()
    {
        if (have_grant_)
            return;
        sem_->wait();
        have_grant_ = true;
    }

    void Release()
    {
        if (!have_grant_)
            return;
        sem_->post();
        have_grant_ = false;
    }

    bool TryAcquire()
    {
        if (!have_grant_ && sem_->try_wait())
            have_grant_ = true;
        return have_grant_;
    }

    void MoveTo(SemaphoreGrant& grant)
    {
        grant.Release();
        grant.sem_ = sem_;
        grant.have_grant_ = have_grant_;
        have_grant_ = false;
    }

    operator bool() const
    {
        return have_grant_;
    }

private:
    Semaphore *sem_;
    bool have_grant_;
};


#endif // BTCLITE_SYNC_H
