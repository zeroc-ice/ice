// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_UTIL_STATICRECMUTEX_H
#define ICE_UTIL_STATICRECMUTEX_H

#include <IceUtil/Config.h>
#include <IceUtil/Lock.h>
#include <IceUtil/ThreadException.h>

namespace IceUtil
{

//
// Forward declaration for friend.
//
class Cond;

class ICE_UTIL_API StaticRecMutex
{
public:

#ifndef _WIN32
    ~StaticRecMutex();
#endif

    //
    // Lock & TryLock typedefs.
    //
    typedef LockT<StaticRecMutex> Lock;
    typedef TryLockT<StaticRecMutex> TryLock;

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
#   endif
#else
#   ifdef __sun
    mutable bool            _mutexInitialized;
#   endif
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

#if defined(_WIN32) || defined(__sun)
    void initialize() const;
#endif

#ifndef _MSC_VER
    friend class Cond;
#endif

};

#if defined(_WIN32) || defined(__sun)
#   define ICE_STATIC_RECMUTEX_INITIALIZER { false }
#else
#   define ICE_STATIC_RECMUTEX_INITIALIZER { PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP }
#endif

//
// For performance reasons the following functions are inlined.
//

#ifdef _WIN32

#   if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0400

inline void
StaticRecMutex::lock() const
{
    if (!_mutexInitialized)
    {
	initialize();
    }
    EnterCriticalSection(_mutex);
}

inline bool
StaticRecMutex::tryLock() const
{
    if (!_mutexInitialized)
    {
	initialize();
    }
    if(!TryEnterCriticalSection(_mutex))
    {
	return false;
    }
    return true;
}

inline void
StaticRecMutex::unlock() const
{
    assert(_mutexInitialized);
    LeaveCriticalSection(_mutex);
}

inline void
StaticRecMutex::unlock(LockState& state) const
{
    assert(_mutexInitialized);
    LeaveCriticalSection(_mutex);
}

inline void
StaticRecMutex::lock(LockState&) const
{
    if (!_mutexInitialized)
    {
	initialize();
    }
    EnterCriticalSection(_mutex);
}

#    else

inline void
StaticRecMutex::lock() const
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
}

inline bool
StaticRecMutex::tryLock() const
{
    if (!_mutexInitialized)
    {
	initialize();
    }
    return WaitForSingleObject(_mutex, 0) == WAIT_OBJECT_0;
}

inline void
StaticRecMutex::unlock() const
{
    if(ReleaseMutex(_mutex) == 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

inline void
StaticRecMutex::unlock(LockState& state) const
{
    unlock();
}

inline void
StaticRecMutex::lock(LockState&) const
{
    lock();
}

#    endif

#else

inline
StaticRecMutex::~StaticRecMutex()
{
    int rc = pthread_mutex_destroy(&_mutex);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

inline void
StaticRecMutex::lock() const
{
#ifdef __sun
    if (!_mutexInitialized)
    {
	initialize();
    }
#endif
    int rc = pthread_mutex_lock(&_mutex);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

inline bool
StaticRecMutex::tryLock() const
{
#ifdef __sun
    if (!_mutexInitialized)
    {
	initialize();
    }
#endif
    int rc = pthread_mutex_trylock(&_mutex);
    if(rc != 0 && rc != EBUSY)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
    return (rc == 0);
}

inline void
StaticRecMutex::unlock() const
{
#ifdef __sun
    assert(_mutexInitialized);
#endif
    int rc = pthread_mutex_unlock(&_mutex);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

inline void
StaticRecMutex::unlock(LockState& state) const
{
#ifdef __sun
    assert(_mutexInitialized);
#endif
    state.mutex = &_mutex;
}

inline void
StaticRecMutex::lock(LockState&) const
{
#ifdef __sun
    if (!_mutexInitialized)
    {
	initialize();
    }
#endif
}

#endif    

} // End namespace IceUtil

#endif
