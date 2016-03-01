// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_ATOMIC_H
#define ICE_UTIL_ATOMIC_H

#if ((defined(ICE_CPP11) && defined(_MSC_VER) && (_MSC_VER > 1600)) || \
     (defined(ICE_CPP11) && !defined(_MSC_VER)))
#   define ICE_CPP11_HAS_ATOMIC
#endif

#if defined(ICE_CPP11_HAS_ATOMIC)
#   include <atomic>
#elif defined(ICE_USE_MUTEX_SHARED)

#   include <IceUtil/Mutex.h>

// Using the gcc builtins requires gcc 4.1 or better. For Linux, i386
// doesn't work. Apple is supported for all architectures. Sun only
// supports sparc (32 and 64 bit).

#elif ((defined(__GNUC__) && (((__GNUC__* 100) + __GNUC_MINOR__) >= 401)) || __clang__)  &&                         \
        ((defined(__sun) && (defined(__sparc) || defined(__sparcv9))) || \
         defined(__APPLE__) || \
        (defined(__linux) && \
                (defined(__i486) || defined(__i586) || \
                 defined(__i686) || defined(__x86_64))))

#   define ICE_HAS_GCC_BUILTINS

#elif defined(_WIN32)
// Nothing to include
#else
// Use a simple mutex
#   include <IceUtil/Mutex.h>
#endif


namespace IceUtilInternal
{

#ifdef ICE_CPP11_HAS_ATOMIC
typedef std::atomic<int> Atomic;
#else

#if defined(_WIN32)
//
// volatile here is required by InterlockedExchangeXXX
// family functions.
//
#  if defined(__MINGW32__) || (defined(_MSC_VER) && (_MSC_VER <= 1500))
typedef volatile LONG ATOMIC_T;
#  else
typedef unsigned int ATOMIC_T;
#  endif
#else
typedef int ATOMIC_T;
#endif


//
// This is temporary and very partial placeholder for std::atomic,
// which is not yet widely available.
//
class ICE_UTIL_API Atomic : public IceUtil::noncopyable
{
public:

    Atomic() :
        _ref(0)
    {
    }

    Atomic(int desired) :
        _ref(desired)
    {
    }

    inline int fetch_add(int value)
    {
#if defined(_WIN32)
        return InterlockedExchangeAdd(&_ref, value);
#elif defined(ICE_HAS_GCC_BUILTINS)
        return __sync_fetch_and_add(&_ref, value);
#else
        IceUtil::Mutex::Lock sync(_mutex);
        int tmp = _ref;
        _ref += value;
        return tmp;
#endif
    }

    inline int fetch_sub(int value)
    {
#if defined(_WIN32)
#  if defined(__MINGW32__) || (defined(_MSC_VER) && (_MSC_VER <= 1500))
        return InterlockedExchangeAdd(&_ref, -value);
#  else
        return InterlockedExchangeSubtract(&_ref, value);
#endif
#elif defined(ICE_HAS_GCC_BUILTINS)
        return __sync_fetch_and_sub(&_ref, value);
#else
        IceUtil::Mutex::Lock sync(_mutex);
        ATOMIC_T tmp = _ref;
        _ref -= value;
        return tmp;
#endif
    }

    inline int load() const
    {
#if defined(_WIN32)
        return InterlockedExchangeAdd(const_cast<ATOMIC_T*>(&_ref), 0);
#elif defined(ICE_HAS_GCC_BUILTINS)
        return __sync_fetch_and_add(const_cast<ATOMIC_T*>(&_ref), 0);
#else
        IceUtil::Mutex::Lock sync(_mutex);
        return _ref;
#endif
    }

    inline int exchange(int value)
    {
#if defined(_WIN32)
        return InterlockedExchange(&_ref, value);
#elif defined(ICE_HAS_GCC_BUILTINS)
        __sync_synchronize();
        return __sync_lock_test_and_set(&_ref, value);
#else
        IceUtil::Mutex::Lock sync(_mutex);
        int tmp = _ref;
        _ref = value;
        return tmp;
#endif
    }

    inline int operator++()
    {
        return fetch_add(1) + 1;
    }

    inline int operator--()
    {
        return fetch_sub(1) - 1;
    }

    inline int operator++(int)
    {
        return fetch_add(1);
    }

    inline int operator--(int)
    {
        return fetch_sub(1);
    }

    inline operator int()
    {
        return load();
    }

    inline operator int() const
    {
        return load();
    }

private:

    ATOMIC_T _ref;
#if !defined(_WIN32) && !defined(ICE_HAS_GCC_BUILTINS)
    IceUtil::Mutex _mutex;
#endif
};

#endif

}

#endif
