
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_MUTEX_H
#define ICE_UTIL_MUTEX_H

#include <IceUtil/Config.h>
#include <IceUtil/Lock.h>
#include <IceUtil/ThreadException.h>
#include <IceUtil/MutexProtocol.h>

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
class ICE_API Mutex
{
public:

    //
    // Lock & TryLock typedefs.
    //
    typedef LockT<Mutex> Lock;
    typedef TryLockT<Mutex> TryLock;

    inline Mutex();
    inline Mutex(MutexProtocol);
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

    inline void init(MutexProtocol);
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
#   ifdef ICE_HAS_WIN32_CONDVAR
        CRITICAL_SECTION* mutex;
#   endif
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
inline
Mutex::Mutex()
{
#ifdef _WIN32
    init(PrioNone);
#else
    init(getDefaultMutexProtocol());
#endif
}

#ifdef _WIN32
inline
Mutex::Mutex(MutexProtocol)
{
    init(PrioNone);
}
#else
inline
Mutex::Mutex(MutexProtocol protocol)
{
    init(protocol);
}
#endif

#ifdef _WIN32

inline void
Mutex::init(MutexProtocol)
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
#ifndef NDEBUG
    if(_mutex.RecursionCount > 1)
    {
        LeaveCriticalSection(&_mutex);
        throw ThreadLockedException(__FILE__, __LINE__);
    }
#endif
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

#  ifdef ICE_HAS_WIN32_CONDVAR
inline void
Mutex::unlock(LockState& state) const
{
    state.mutex = &_mutex;
}

inline void
Mutex::lock(LockState&) const
{
}
#  else
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
#   endif

#else

inline void
Mutex::init(MutexProtocol
#if defined(_POSIX_THREAD_PRIO_INHERIT) && _POSIX_THREAD_PRIO_INHERIT > 0
            protocol
#endif
            )
{
    int rc;
    pthread_mutexattr_t attr;
    rc = pthread_mutexattr_init(&attr);
    assert(rc == 0);
    if(rc != 0)
    {
        pthread_mutexattr_destroy(&attr);
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }

    //
    // Enable mutex error checking
    //
    rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    assert(rc == 0);
    if(rc != 0)
    {
        pthread_mutexattr_destroy(&attr);
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }

    //
    // If system has support for priority inheritance we set the protocol
    // attribute of the mutex
    //
#if defined(_POSIX_THREAD_PRIO_INHERIT) && _POSIX_THREAD_PRIO_INHERIT > 0
    if(PrioInherit == protocol)
    {
        rc = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
        if(rc != 0)
        {
            pthread_mutexattr_destroy(&attr);
            throw ThreadSyscallException(__FILE__, __LINE__, rc);
        }
    }
#endif

    rc = pthread_mutex_init(&_mutex, &attr);
    assert(rc == 0);
    if(rc != 0)
    {
        pthread_mutexattr_destroy(&attr);
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }

    rc = pthread_mutexattr_destroy(&attr);
    assert(rc == 0);
    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

inline
Mutex::~Mutex()
{
#ifndef NDEBUG
    int rc = 0;
    rc = pthread_mutex_destroy(&_mutex);
    assert(rc == 0);
#else
    pthread_mutex_destroy(&_mutex);
#endif
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
