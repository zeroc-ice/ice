// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_UTIL_RMUTEX_H
#define ICE_UTIL_RMUTEX_H

#include <IceUtil/Config.h>
#include <IceUtil/Lock.h>

namespace IceUtil
{

//
// Forward declarations for friend.
//
class Cond;

//
// Recursive Mutex implementation.
//
class ICE_UTIL_API RecMutex
{
public:

    //
    // Lock & TryLock typedefs.
    //
    typedef Lock<RecMutex> Lock;
    typedef TryLock<RecMutex> TryLock;

    typedef ConstLock<RecMutex> ConstLock;
    typedef ConstTryLock<RecMutex> ConstTryLock;

    RecMutex();
    ~RecMutex();

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
    RecMutex(const RecMutex&);
    void operator=(const RecMutex&);

    //
    // LockState and the lock/unlock variations are for use by the
    // Condition variable implementation.
    //
#ifdef WIN32
    struct LockState
    {
	int count;
    };
#else
    struct LockState
    {
	pthread_mutex_t* mutex;
	int count;
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

    int _count;
};

} // End namespace IceUtil

#endif
