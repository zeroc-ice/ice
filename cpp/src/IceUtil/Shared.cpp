// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Shared.h>

using namespace IceUtil;

#ifdef ICE_HAS_ATOMIC_FUNCTIONS

namespace IceUtilInternal
{

//
// Linux only. Unfortunately, asm/atomic.h builds non-SMP safe code
// with non-SMP kernels. This means that executables compiled with a
// non-SMP kernel would fail randomly due to concurrency errors with
// reference counting on SMP hosts. Therefore the relevent pieces of
// atomic.h are more-or-less duplicated.
//

/*
 * atomicInc - increment ice_atomic variable
 * @v: pointer of type AtomicCounter
 * 
 * Atomically increments @v by 1. Note that the guaranteed useful
 * range of an AtomicCounter is only 24 bits.
 *
 * Inlined because this operation is performance critical.
 */
static inline void atomicInc(volatile int* counter)
{
    __asm__ __volatile__(
        "lock ; incl %0"
        :"=m" (*counter)
        :"m" (*counter));
}

/**
 * atomicDecAndTest - decrement and test
 * @v: pointer of type AtomicCounter
 * 
 * Atomically decrements @v by 1 and returns true if the result is 0,
 * or false for all other cases. Note that the guaranteed useful
 * range of an AtomicCounter is only 24 bits.
 *
 * Inlined because this operation is performance critical.
 */
static inline int atomicDecAndTest(volatile int* counter)
{
    unsigned char c;
    __asm__ __volatile__(
        "lock ; decl %0; sete %1"
        :"=m" (*counter), "=qm" (c)
        :"m" (*counter) : "memory");
    return c != 0;
}

/**
 * atomicExchangeAdd - same as InterlockedExchangeAdd. This
 * didn't come from atomic.h (the code was derived from similar code
 * in /usr/include/asm/rwsem.h)
 *
 * Inlined because this operation is performance critical.
 */
static inline int atomicExchangeAdd(volatile int* counter, int i)
{
    int tmp = i;
    __asm__ __volatile__(
        "lock ; xadd %0,(%2)"
        :"+r"(tmp), "=m"(*counter)
        :"r"(counter), "m"(*counter)
        : "memory");
    return tmp + i;
}

}

#endif


IceUtil::SimpleShared::SimpleShared() :
    _ref(0),
    _noDelete(false)
{
}

IceUtil::SimpleShared::SimpleShared(const SimpleShared&) :
    _ref(0),
    _noDelete(false)
{
}

IceUtil::Shared::Shared() :
    _ref(0),
    _noDelete(false)
{
}

IceUtil::Shared::Shared(const Shared&) :
    _ref(0),
    _noDelete(false)
{
}

void 
IceUtil::Shared::__incRef()
{
#if defined(_WIN32)
    assert(InterlockedExchangeAdd(&_ref, 0) >= 0);
    InterlockedIncrement(&_ref);
#elif defined(ICE_HAS_GCC_BUILTINS)

#   ifndef NDEBUG
    int c = 
#   endif
        __sync_fetch_and_add(&_ref, 1);
    assert(c >= 0);
#elif defined(ICE_HAS_ATOMIC_FUNCTIONS)
    assert(IceUtilInternal::atomicExchangeAdd(&_ref, 0) >= 0);
    IceUtilInternal::atomicInc(&_ref);
#else
    _mutex.lock();
    assert(_ref >= 0);
    ++_ref;
    _mutex.unlock();
#endif
}

void 
IceUtil::Shared::__decRef()
{
#if defined(_WIN32)
    assert(InterlockedExchangeAdd(&_ref, 0) > 0);
    if(InterlockedDecrement(&_ref) == 0 && !_noDelete)
    {
        _noDelete = true;
        delete this;
    }
#elif defined(ICE_HAS_GCC_BUILTINS)
    int c = __sync_fetch_and_sub(&_ref, 1);
    assert(c > 0);
    if(c == 1 && !_noDelete)
    {
        _noDelete = true;
        delete this;
    }
#elif defined(ICE_HAS_ATOMIC_FUNCTIONS)
    assert(IceUtilInternal::atomicExchangeAdd(&_ref, 0) > 0);
    if(IceUtilInternal::atomicDecAndTest(&_ref) && !_noDelete)
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

int
IceUtil::Shared::__getRef() const
{
#if defined(_WIN32)
    return InterlockedExchangeAdd(const_cast<LONG*>(&_ref), 0);
#elif defined(ICE_HAS_GCC_BUILTINS)
    return __sync_fetch_and_sub(const_cast<volatile int*>(&_ref), 0);
#elif defined(ICE_HAS_ATOMIC_FUNCTIONS)
    return IceUtilInternal::atomicExchangeAdd(const_cast<volatile int*>(&_ref), 0);
#else
    _mutex.lock();
    int ref = _ref;
    _mutex.unlock();
    return ref;
#endif
}

void
IceUtil::Shared::__setNoDelete(bool b)
{
    _noDelete = b;
}
