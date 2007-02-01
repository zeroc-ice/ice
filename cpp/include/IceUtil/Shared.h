// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_SHARED_H
#define ICE_UTIL_SHARED_H

#include <IceUtil/Config.h>

#if defined(ICE_USE_MUTEX_SHARED)
#   include <IceUtil/Mutex.h>

#elif (defined(__linux) || defined(__FreeBSD__)) && (defined(__i386) || defined(__x86_64)) && !defined(__ICC)
#   define ICE_HAS_ATOMIC_FUNCTIONS

// __ICC: The inline assembler causes problems with shared libraries.
//
// Linux only. Unfortunately, asm/atomic.h builds non-SMP safe code
// with non-SMP kernels. This means that executables compiled with a
// non-SMP kernel would fail randomly due to concurrency errors with
// reference counting on SMP hosts. Therefore the relevent pieces of
// atomic.h are more-or-less duplicated.
//

/*
 * Make sure gcc doesn't try to be clever and move things around
 * on us. We need to use _exactly_ the address the user gave us,
 * not some alias that contains the same information.
 */
struct ice_atomic_t
{
    volatile int counter;
};

/*
 * ice_atomic_set - set ice_atomic variable
 * @v: pointer of type ice_atomic_t
 * @i: required value
 * 
 * Atomically sets the value of @v to @i. Note that the guaranteed
 * useful range of an ice_atomic_t is only 24 bits.
 */
inline void ice_atomic_set(ice_atomic_t* v, int i)
{
    v->counter = i;
}

/*
 * ice_atomic_inc - increment ice_atomic variable
 * @v: pointer of type ice_atomic_t
 * 
 * Atomically increments @v by 1. Note that the guaranteed useful
 * range of an ice_atomic_t is only 24 bits.
 *
 * Inlined because this operation is performance critical.
 */
inline void ice_atomic_inc(ice_atomic_t *v)
{
    __asm__ __volatile__(
        "lock ; incl %0"
        :"=m" (v->counter)
        :"m" (v->counter));
}

/**
 * ice_atomic_dec_and_test - decrement and test
 * @v: pointer of type ice_atomic_t
 * 
 * Atomically decrements @v by 1 and returns true if the result is 0,
 * or false for all other cases. Note that the guaranteed useful
 * range of an ice_atomic_t is only 24 bits.
 *
 * Inlined because this operation is performance critical.
 */
inline int ice_atomic_dec_and_test(ice_atomic_t *v)
{
    unsigned char c;
    __asm__ __volatile__(
        "lock ; decl %0; sete %1"
        :"=m" (v->counter), "=qm" (c)
        :"m" (v->counter) : "memory");
    return c != 0;
}

/**
 * ice_atomic_exchange_add - same as InterlockedExchangeAdd. This
 * didn't come from atomic.h (the code was derived from similar code
 * in /usr/include/asm/rwsem.h)
 *
 * Inlined because this operation is performance critical.
 */
inline int ice_atomic_exchange_add(int i, ice_atomic_t* v)
{
    int tmp = i;
    __asm__ __volatile__(
        "lock ; xadd %0,(%2)"
        :"+r"(tmp), "=m"(v->counter)
        :"r"(v), "m"(v->counter)
        : "memory");
    return tmp + i;
}

#elif defined(_WIN32)
// Nothing to include
#else
// Use a simple mutex
#   include <IceUtil/Mutex.h>
#endif

//
// Base classes for reference counted types. The IceUtil::Handle
// template can be used for smart pointers to types derived from these
// bases.
//
// IceUtil::SimpleShared
// =====================
//
// A non thread-safe base class for reference-counted types.
//
// IceUtil::Shared
// ===============
//
// A thread-safe base class for reference-counted types.
//
namespace IceUtil
{

class ICE_UTIL_API SimpleShared
{
public:

    SimpleShared();
    SimpleShared(const SimpleShared&);

    virtual ~SimpleShared()
    {
    }

    SimpleShared& operator=(const SimpleShared&)
    {
        return *this;
    }

    void __incRef()
    {
        assert(_ref >= 0);
        ++_ref;
    }

    void __decRef()
    {
        assert(_ref > 0);
        if(--_ref == 0)
        {
            if(!_noDelete)
            {
                _noDelete = true;
                delete this;
            }
        }
    }

    int __getRef() const
    {
        return _ref;
    }

    void __setNoDelete(bool b)
    {
        _noDelete = b;
    }

private:

    int _ref;
    bool _noDelete;
};

class ICE_UTIL_API Shared
{
public:

    Shared();
    Shared(const Shared&);

    virtual ~Shared()
    {
    }

    Shared& operator=(const Shared&)
    {
        return *this;
    }

    void __incRef()
    {
#if defined(_WIN32)
        assert(InterlockedExchangeAdd(&_ref, 0) >= 0);
        InterlockedIncrement(&_ref);
#elif defined(ICE_HAS_ATOMIC_FUNCTIONS)
        assert(ice_atomic_exchange_add(0, &_ref) >= 0);
        ice_atomic_inc(&_ref);
#else
        _mutex.lock();
        assert(_ref >= 0);
        ++_ref;
        _mutex.unlock();
#endif
    }

    void __decRef()
    {
#if defined(_WIN32)
        assert(InterlockedExchangeAdd(&_ref, 0) > 0);
        if(InterlockedDecrement(&_ref) == 0 && !_noDelete)
        {
            _noDelete = true;
            delete this;
        }
#elif defined(ICE_HAS_ATOMIC_FUNCTIONS)
        assert(ice_atomic_exchange_add(0, &_ref) > 0);
        if(ice_atomic_dec_and_test(&_ref) && !_noDelete)
        {
            _noDelete = true;
            delete this;
        }
#else
        _mutex.lock();
        bool doDelete = false;
        assert(_ref > 0);
        if(--_ref == 0)
        {
            doDelete = !_noDelete;
            _noDelete = true;
        }
        _mutex.unlock();
        if(doDelete)
        {
            delete this;
        }
#endif
    }

    int __getRef() const;
    void __setNoDelete(bool);

protected:

#if defined(_WIN32)
    LONG _ref;
#elif defined(ICE_HAS_ATOMIC_FUNCTIONS)
    ice_atomic_t _ref;
#else
    int _ref;
    Mutex _mutex;
#endif
    bool _noDelete;
};

}

#endif
