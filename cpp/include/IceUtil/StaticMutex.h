// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_STATIC_MUTEX_H
#define ICE_UTIL_STATIC_MUTEX_H

#include <IceUtil/Config.h>
#include <IceUtil/Lock.h>
#include <IceUtil/ThreadException.h>

namespace IceUtil
{

//
// Forward declaration for friend.
//
class Cond;

//
// Simple non-recursive Mutex implementation.
// These mutexes are POD types (see ISO C++ 9(4) and 8.5.1) and must be
// initialized statically using ICE_STATIC_MUTEX_INITIALIZER.
//

//
class StaticMutex
{
public:

    //
    // Lock & TryLock typedefs.
    //
    typedef LockT<StaticMutex> Lock;
    typedef TryLockT<StaticMutex> TryLock;

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


#ifdef _WIN32
    mutable bool             _mutexInitialized;
#   if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0400
    mutable CRITICAL_SECTION* _mutex;
#   else
    mutable HANDLE _mutex;
    mutable int _recursionCount;
#   endif
#else
    mutable pthread_mutex_t _mutex;
#endif



#ifndef _MSC_VER
// COMPILERBUG
// VC++ considers that aggregates should not have private members ...
// even if it's just functions.
private:
#endif

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

#ifdef _WIN32
    ICE_UTIL_API void initialize() const;
#endif

#ifndef _MSC_VER
    friend class Cond;
#endif

};

#ifdef _WIN32
#   define ICE_STATIC_MUTEX_INITIALIZER { false }
#else
#   define ICE_STATIC_MUTEX_INITIALIZER { PTHREAD_MUTEX_INITIALIZER }
#endif


// A "shared" global mutex that can be used for very simple tasks
// which should not lock any other mutexes.
//
extern ICE_UTIL_API StaticMutex globalMutex;

//
// For performance reasons the following functions are inlined.
//

#ifdef _WIN32

#   if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0400

inline void
StaticMutex::lock() const
{
    if (!_mutexInitialized)
    {
	initialize();
    }
    EnterCriticalSection(_mutex);
    assert(_mutex->RecursionCount == 1);
}

inline bool
StaticMutex::tryLock() const
{
    if (!_mutexInitialized)
    {
	initialize();
    }
    if(!TryEnterCriticalSection(_mutex))
    {
	return false;
    }
    if(_mutex->RecursionCount > 1)
    {
	LeaveCriticalSection(_mutex);
	return false;
    }
    return true;
}

inline void
StaticMutex::unlock() const
{
    assert(_mutexInitialized);
    assert(_mutex->RecursionCount == 1);
    LeaveCriticalSection(_mutex);
}

inline void
StaticMutex::unlock(LockState&) const
{
    assert(_mutexInitialized);
    LeaveCriticalSection(_mutex);
}

inline void
StaticMutex::lock(LockState&) const
{
    if (!_mutexInitialized)
    {
	initialize();
    }
    EnterCriticalSection(_mutex);
}

#    else

inline void
StaticMutex::lock() const
{
    if (!_mutexInitialized)
    {
	initialize();
    }

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
    assert(_recursionCount == 1);
}

inline bool
StaticMutex::tryLock() const
{
    if (!_mutexInitialized)
    {
	initialize();
    }

    DWORD rc = WaitForSingleObject(_mutex, 0);
    if(rc != WAIT_OBJECT_0)
    {
	return false;
    }
    else if(_recursionCount == 1)
    {
	_recursionCount++;
	unlock();
	return false;
    }
    else
    {
	_recursionCount++;
	return true;
    }
}

inline void
StaticMutex::unlock() const
{
    _recursionCount--;
    BOOL rc = ReleaseMutex(_mutex);
    if(rc == 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

inline void
StaticMutex::unlock(LockState& state) const
{
    unlock();
}

inline void
StaticMutex::lock(LockState&) const
{
    lock();
}

#    endif

#else

inline void
StaticMutex::lock() const
{
    int rc = pthread_mutex_lock(&_mutex);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

inline bool
StaticMutex::tryLock() const
{
    int rc = pthread_mutex_trylock(&_mutex);
    if(rc != 0 && rc != EBUSY)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
    return (rc == 0);
}

inline void
StaticMutex::unlock() const
{
    int rc = pthread_mutex_unlock(&_mutex);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

inline void
StaticMutex::unlock(LockState& state) const
{
    state.mutex = &_mutex;
}

inline void
StaticMutex::lock(LockState&) const
{
}

#endif    

} // End namespace IceUtil

#endif
