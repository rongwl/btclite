#ifndef BTCLITE_SYNC_H
#define BTCLITE_SYNC_H

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
class CMutexLock {
public:
	CMutexLock(Mutex& mutex, const char *psz_name, const char *psz_file, int line, bool try_lock = false)
		: lock(mutex, std::defer_lock)
	{
		if (try_lock) 
			TryEnter(psz_name, psz_file, line);
		else
			Enter(psz_name, psz_file, line);
	}

	CMutexLock(Mutex *pmutex, const char *psz_name, const char *psz_file, int line, bool try_lock = false)
	{
		if (!pmutex)
			return;
		lock = std::unique_lock<Mutex>(*pmutex, std::defer_lock);
		if (try_lock)
			TryEnter(psz_name, psz_file, line);
		else
			Enter(psz_name, psz_file, line);
	}

	~CMutexLock() 
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

using CriticalBlock = CMutexLock<CriticalSection>;

#define PASTE(x, y) x ## y
#define PASTE2(x, y) PASTE(x, y)

#define LOCK(cs) CriticalBlock PASTE2(criticalblock, __COUNTER__)(cs, #cs, __FILE__, __LINE__)
#define TRY_LOCK(cs, name) CriticalBlock name(cs, #cs, __FILE__, __LINE__, true)

#endif // BTCLITE_SYNC_H
