// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_UTIL_MUTEX_H
#define ICE_UTIL_MUTEX_H

#include <IceUtil/Config.h>
#include <IceUtil/Exception.h> // Necessary for inline functions
#include <IceUtil/Lock.h>

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
    typedef Lock<Mutex> Lock;
    typedef TryLock<Mutex> TryLock;

    typedef ConstLock<Mutex> ConstLock;
    typedef ConstTryLock<Mutex> ConstTryLock;

    Mutex();
    ~Mutex();

    //
    // Note that lock/trylock & unlock in general should not be used
    // directly. Instead use Lock & TryLock.
    //

    //
    // The boolean values are for the Monitor implementation which
    // needs to know whether the Mutex has been locked for the first
    // time, or unlocked for the last time (that is another thread is
    // able to acquire the mutex).
    //

    //
    // Return true if the mutex has been locked for the first time.
    //
    bool lock();

    //
    // Throw LockedException in the case that the lock call would
    // block (that is the mutex is already owned by some other
    // thread). Returns true if the mutex has been locked for the
    // first time.
    //
    bool trylock();

    //
    // Returns true if the mutex has been unlocked for the last time
    // (false otherwise).
    //
    bool unlock();

private:

    // noncopyable
    Mutex(const Mutex&);
    void operator=(const Mutex&);

    //
    // LockState and the lock/unlock variations are for use by the
    // Condition variable implementation.
    //
#ifdef WIN32
    struct LockState
    {
    };
#else
    struct LockState
    {
	pthread_mutex_t* mutex;
    };
#endif

    void unlock(LockState&);
    void lock(LockState&);

    friend class Cond;

#ifdef WIN32
    CRITICAL_SECTION _mutex;
#else
    pthread_mutex_t _mutex;
#endif
};

#ifdef WIN32
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

inline bool
Mutex::lock()
{
    EnterCriticalSection(&_mutex);
    //
    // If necessary this can be removed and replaced with a _count
    // member (like the UNIX implementation of RecMutex).
    //
    assert(_mutex.RecursionCount == 1);
    return true;
}

inline bool
Mutex::trylock()
{
    if (!TryEnterCriticalSection(&_mutex))
    {
	throw LockedException(__FILE__, __LINE__);
    }
    if(_mutex.RecursionCount > 1)
    {
	LeaveCriticalSection(&_mutex);
	throw LockedException(__FILE__, __LINE__);
    }
    return true;
}

inline bool
Mutex::unlock()
{
    assert(_mutex.RecursionCount == 1);
    LeaveCriticalSection(&_mutex);
    return true;
}

inline void
Mutex::unlock(LockState& state)
{
    LeaveCriticalSection(&_mutex);
}

inline void
Mutex::lock(LockState&)
{
    EnterCriticalSection(&_mutex);
}

#else

inline
Mutex::Mutex()
{
    int rc = pthread_mutex_init(&_mutex, 0);
    if (rc != 0)
    {
	throw SyscallException(strerror(rc), __FILE__, __LINE__);
    }
}

inline
Mutex::~Mutex()
{
    int rc = 0;
    rc = pthread_mutex_destroy(&_mutex);
    assert(rc == 0);
}

inline bool
Mutex::lock()
{
    int rc = pthread_mutex_lock(&_mutex);
    if (rc != 0)
    {
	throw SyscallException(strerror(rc), __FILE__, __LINE__);
    }
    return true;
}

inline bool
Mutex::trylock()
{
    int rc = pthread_mutex_trylock(&_mutex);
    if (rc != 0)
    {
	if (rc == EBUSY)
	{
	    throw LockedException(__FILE__, __LINE__);
	}
	throw SyscallException(strerror(rc), __FILE__, __LINE__);
    }
    return true;
}

inline bool
Mutex::unlock()
{
    int rc = pthread_mutex_unlock(&_mutex);
    if (rc != 0)
    {
	throw SyscallException(strerror(rc), __FILE__, __LINE__);
    }
    return true;
}

inline void
Mutex::unlock(LockState& state)
{
    state.mutex = &_mutex;
}

inline void
Mutex::lock(LockState&)
{
}
#endif    

} // End namespace IceUtil

#endif
