//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_RMUTEX_H
#define ICE_UTIL_RMUTEX_H

#include <IceUtil/Config.h>
#include <IceUtil/Lock.h>
#include <IceUtil/ThreadException.h>
#include <IceUtil/MutexProtocol.h>

namespace IceUtil
{

//
// Forward declarations for friend.
//
class Cond;

//
// Recursive Mutex implementation.
//
class ICE_API RecMutex
{
public:

    //
    // Lock & TryLock typedefs.
    //
    typedef LockT<RecMutex> Lock;
    typedef TryLockT<RecMutex> TryLock;

    RecMutex();
    RecMutex(const MutexProtocol);
    ~RecMutex();

    //
    // Note that lock/tryLock & unlock in general should not be used
    // directly. Instead use Lock & TryLock.
    //

    void lock() const;

    //
    // Returns true if the lock was acquired or was already acquired
    // by the calling thread, and false otherwise.
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

    void init(const MutexProtocol);
    // noncopyable
    RecMutex(const RecMutex&);
    void operator=(const RecMutex&);

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
        int count;
    };
#else
    struct LockState
    {
        pthread_mutex_t* mutex;
        int count;
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

    mutable int _count;
};

} // End namespace IceUtil

#endif
