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

#ifndef ICE_UTIL_MUTEX_H
#define ICE_UTIL_MUTEX_H

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
//
// Don't use noncopyable otherwise you end up with warnings like this:
//
// In file included from Connection.cpp:20:
// ../../include/Ice/Outgoing.h:88: warning: direct base
// `IceUtil::noncopyable' inaccessible in `IceInternal::Outgoing' due
// to ambiguity
//
class ICE_UTIL_API Mutex
{
public:

    //
    // Lock & TryLock typedefs.
    //
    typedef LockT<Mutex> Lock;
    typedef TryLockT<Mutex> TryLock;

    Mutex();
    ~Mutex();

    //
    // Note that lock/trylock & unlock in general should not be used
    // directly. Instead use Lock & TryLock.
    //
 
    void lock() const;

    //
    // Returns true if the lock was acquired, and false otherwise.
    //
    bool trylock() const;

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
    mutable CRITICAL_SECTION _mutex;
#else
    mutable pthread_mutex_t _mutex;
#endif
};

//
// For performance reasons the following functions are inlined.
//

#ifdef _WIN32

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
    //
    // If necessary this can be removed and replaced with a _count
    // member (like the UNIX implementation of RecMutex).
    //
    assert(_mutex.RecursionCount == 1);
}

inline bool
Mutex::trylock() const
{
    if(!TryEnterCriticalSection(&_mutex))
    {
	return false;
    }
    if(_mutex.RecursionCount > 1)
    {
	LeaveCriticalSection(&_mutex);
	return false;
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
Mutex::unlock(LockState& state) const
{
    LeaveCriticalSection(&_mutex);
}

inline void
Mutex::lock(LockState&) const
{
    EnterCriticalSection(&_mutex);
}

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
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

inline bool
Mutex::trylock() const
{
    int rc = pthread_mutex_trylock(&_mutex);
    if(rc != 0 && rc != EBUSY)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
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
