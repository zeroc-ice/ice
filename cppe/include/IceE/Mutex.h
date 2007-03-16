// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_MUTEX_H
#define ICEE_MUTEX_H

#include <IceE/Config.h>
#include <IceE/Lock.h>
#include <IceE/ThreadException.h>

namespace IceUtil
{

//
// Forward declaration for friend.
//
class Cond;

//
// Simple non-recursive Mutex implementation.
//
// Don't use noncopyable otherwise you end up with warnings like this:
//
// In file included from Connection.cpp:20:
// ../../include/IceE/Outgoing.h:88: warning: direct base
// `IceUtil::noncopyable' inaccessible in `IceInternal::Outgoing' due
// to ambiguity
//
class Mutex
{
public:

    //
    // Lock & TryLock typedefs.
    //
    typedef LockT<Mutex> Lock;
    typedef TryLockT<Mutex> TryLock;

    inline Mutex();
    ~Mutex();

    //
    // Note that lock/tryLock & unlock in general should not be used
    // directly. Instead use Lock & TryLock.
    //
 
    void lock() const;

    //
    // Returns true if the lock was acquired, and false otherwise.
    //
    bool tryLock() const;

    void unlock() const;

    //
    // Returns true if the mutex will unlock when calling unlock()
    // (false otherwise). For non-recursive mutexes, this will always
    // return true. 
    // This function is used by the Monitor implementation to know whether 
    // the Mutex has been locked for the first time, or unlocked for the 
    // last time (that is another thread is able to acquire the mutex).
    // Pre-condition: the mutex must be locked.
    //
    bool willUnlock() const;

private:

    // noncopyable
    Mutex(const Mutex&);
    void operator=(const Mutex&);

    //
    // LockState and the lock/unlock variations are for use by the
    // Condition variable implementation.
    //
#ifdef _WIN32
    struct LockState
    {
    };
#else
    struct LockState
    {
	pthread_mutex_t* mutex;
    };
#endif

    void unlock(LockState&) const;
    void lock(LockState&) const;

    friend class Cond;

#ifdef _WIN32
# if !defined(_WIN32_WCE) && defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0400
    mutable CRITICAL_SECTION _mutex;
#   else
    mutable HANDLE _mutex;
    mutable int _recursionCount;
#   endif
#else
    mutable pthread_mutex_t _mutex;
#endif
};

//
// For performance reasons the following functions are inlined.
//

#ifdef _WIN32

# if !defined(_WIN32_WCE) && defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0400

inline
Mutex::Mutex()
{
    InitializeCriticalSection(&_mutex);
}

inline
Mutex::~Mutex()
{
    DeleteCriticalSection(&_mutex);
}

inline void
Mutex::lock() const
{
    EnterCriticalSection(&_mutex);
    assert(_mutex.RecursionCount == 1);
}

inline bool
Mutex::tryLock() const
{
    if(!TryEnterCriticalSection(&_mutex))
    {
	return false;
    }
    if(_mutex.RecursionCount > 1)
    {
	LeaveCriticalSection(&_mutex);
        throw ThreadLockedException(__FILE__, __LINE__);
    }
    return true;
}

inline void
Mutex::unlock() const
{
    assert(_mutex.RecursionCount == 1);
    LeaveCriticalSection(&_mutex);
}

inline void
Mutex::unlock(LockState&) const
{
    LeaveCriticalSection(&_mutex);
}

inline void
Mutex::lock(LockState&) const
{
    EnterCriticalSection(&_mutex);
}

#   else

inline
Mutex::Mutex() :
    _recursionCount(0)
{
    _mutex = CreateMutex(0, false, 0);
    if(_mutex == 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

inline
Mutex::~Mutex()
{
    BOOL rc = CloseHandle(_mutex);
    if(rc == 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

inline void
Mutex::lock() const
{
    DWORD rc = WaitForSingleObject(_mutex, INFINITE);
    if(rc != WAIT_OBJECT_0)
    {
	if(rc == WAIT_FAILED)
	{
	    throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
	}
	else
	{
	    throw ThreadSyscallException(__FILE__, __LINE__, 0);
	}
    }
    _recursionCount++;
}

inline bool
Mutex::tryLock() const
{
    DWORD rc = WaitForSingleObject(_mutex, 0);
    if(rc != WAIT_OBJECT_0)
    {
	return false;
    }
    else if(_recursionCount == 1)
    {
	_recursionCount++;
	unlock();
        throw ThreadLockedException(__FILE__, __LINE__);
    }
    else
    {
	_recursionCount++;
	return true;
    }
}

inline void
Mutex::unlock() const
{
    _recursionCount--;
    BOOL rc = ReleaseMutex(_mutex);
    if(rc == 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

inline void
Mutex::unlock(LockState& state) const
{
    unlock();
}

inline void
Mutex::lock(LockState&) const
{
    lock();
}

#   endif

#else

inline
Mutex::Mutex()
{
#ifdef NDEBUG
    int rc = pthread_mutex_init(&_mutex, 0);
#else

    int rc;
#if defined(__linux) && !defined(__USE_UNIX98)
    const pthread_mutexattr_t attr = { PTHREAD_MUTEX_ERRORCHECK_NP };
#else
    pthread_mutexattr_t attr;
    rc = pthread_mutexattr_init(&attr);
    assert(rc == 0);
    rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    assert(rc == 0);
#endif
    rc = pthread_mutex_init(&_mutex, &attr);

#if defined(__linux) && !defined(__USE_UNIX98)
// Nothing to do
#else
    rc = pthread_mutexattr_destroy(&attr);
    assert(rc == 0);
#endif
#endif

    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

inline
Mutex::~Mutex()
{
    int rc = 0;
    rc = pthread_mutex_destroy(&_mutex);
    assert(rc == 0);
}

inline void
Mutex::lock() const
{
    int rc = pthread_mutex_lock(&_mutex);
    if(rc != 0)
    {
        if(rc == EDEADLK)
        {
            throw ThreadLockedException(__FILE__, __LINE__);
        }
        else
        {
            throw ThreadSyscallException(__FILE__, __LINE__, rc);
        }
    }
}

inline bool
Mutex::tryLock() const
{
    int rc = pthread_mutex_trylock(&_mutex);
    if(rc != 0 && rc != EBUSY)
    {
        if(rc == EDEADLK)
        {
            throw ThreadLockedException(__FILE__, __LINE__);
        }
        else
        {
            throw ThreadSyscallException(__FILE__, __LINE__, rc);
        }
    }
    return (rc == 0);
}

inline void
Mutex::unlock() const
{
    int rc = pthread_mutex_unlock(&_mutex);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

inline void
Mutex::unlock(LockState& state) const
{
    state.mutex = &_mutex;
}

inline void
Mutex::lock(LockState&) const
{
}

#endif    

inline bool
Mutex::willUnlock() const
{
    return true;
}

} // End namespace IceUtil

#endif
