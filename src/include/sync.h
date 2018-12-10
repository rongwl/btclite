#ifndef BTCDEMO_SYNC_H
#define BTCDEMO_SYNC_H

#include <mutex>

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

class CCriticalSection : public std::recursive_mutex {
public:
	~CCriticalSection()
	{
		DeleteLock((void*)this);
	}
};

typedef CMutexLock<CCriticalSection> CCriticalBlock;

#define PASTE(x, y) x ## y
#define PASTE2(x, y) PASTE(x, y)

#define LOCK(cs) CCriticalBlock PASTE2(criticalblock, __COUNTER__)(cs, #cs, __FILE__, __LINE__)
#define TRY_LOCK(cs, name) CCriticalBlock name(cs, #cs, __FILE__, __LINE__, true)

#endif // BTCDEMO_SYNC_H
