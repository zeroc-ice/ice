// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
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

#if defined(_MSC_VER) && (_MSC_VER < 1300)
//
// Old versions of the Platform SDK don't have InterlockedCompareExchangePointer
//
#   ifndef InterlockedCompareExchangePointer
#      define InterlockedCompareExchangePointer(Destination, ExChange, Comperand) \
          InterlockedCompareExchange(Destination, ExChange, Comperand)
#   endif
#endif

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
class ICE_UTIL_API StaticMutex
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
 
    ICE_DEPRECATED_API void lock() const;

    //
    // Returns true if the lock was acquired, and false otherwise.
    //
    ICE_DEPRECATED_API bool tryLock() const;

    ICE_DEPRECATED_API void unlock() const;


#ifdef _WIN32
    ICE_DEPRECATED_API mutable CRITICAL_SECTION* _mutex;
#else
    ICE_DEPRECATED_API mutable pthread_mutex_t _mutex;
#endif



#if !defined(_MSC_VER) && !defined(__BCPLUSPLUS__)
// COMPILERBUG
// VC++ BC++ considers that aggregates should not have private members ...
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
    bool initialized() const;
    void initialize() const;
#endif

#ifndef _MSC_VER
    friend class Cond;
#endif

};

#ifdef _WIN32
#   define ICE_STATIC_MUTEX_INITIALIZER { 0 }
#else
#   define ICE_STATIC_MUTEX_INITIALIZER { PTHREAD_MUTEX_INITIALIZER }
#endif

} // End namespace IceUtil

#endif
