// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Shared.h>

namespace IceUtilInternal
{
#ifdef ICE_HAS_ATOMIC_FUNCTIONS

/*
 * atomicSet - set ice_atomic variable
 * @v: pointer of type AtomicCounter
 * @i: required value
 * 
 * Atomically sets the value of @v to @i. Note that the guaranteed
 * useful range of an AtomicCounter is only 24 bits.
 */
inline void atomicSet(AtomicCounter* v, int i)
{
    v->counter = i;
}

/*
 * atomicInc - increment ice_atomic variable
 * @v: pointer of type AtomicCounter
 * 
 * Atomically increments @v by 1. Note that the guaranteed useful
 * range of an AtomicCounter is only 24 bits.
 *
 * Inlined because this operation is performance critical.
 */
inline void atomicInc(AtomicCounter *v)
{
    __asm__ __volatile__(
        "lock ; incl %0"
        :"=m" (v->counter)
        :"m" (v->counter));
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
inline int atomicDecAndTest(AtomicCounter *v)
{
    unsigned char c;
    __asm__ __volatile__(
        "lock ; decl %0; sete %1"
        :"=m" (v->counter), "=qm" (c)
        :"m" (v->counter) : "memory");
    return c != 0;
}

/**
 * atomicExchangeAdd - same as InterlockedExchangeAdd. This
 * didn't come from atomic.h (the code was derived from similar code
 * in /usr/include/asm/rwsem.h)
 *
 * Inlined because this operation is performance critical.
 */
inline int atomicExchangeAdd(int i, AtomicCounter* v)
{
    int tmp = i;
    __asm__ __volatile__(
        "lock ; xadd %0,(%2)"
        :"+r"(tmp), "=m"(v->counter)
        :"r"(v), "m"(v->counter)
        : "memory");
    return tmp + i;
}
#endif
}

using namespace IceUtil;
using namespace IceUtilInternal;

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
#ifndef ICE_HAS_ATOMIC_FUNCTIONS
    _ref(0),
#endif
    _noDelete(false)
{
#ifdef ICE_HAS_ATOMIC_FUNCTIONS
    atomicSet(&_ref, 0);
#endif
}

IceUtil::Shared::Shared(const Shared&) :
#ifndef ICE_HAS_ATOMIC_FUNCTIONS
    _ref(0),
#endif
    _noDelete(false)
{
#ifdef ICE_HAS_ATOMIC_FUNCTIONS
    atomicSet(&_ref, 0);
#endif
}

void 
IceUtil::Shared::__incRef()
{
#if defined(_WIN32)
    assert(InterlockedExchangeAdd(&_ref, 0) >= 0);
    InterlockedIncrement(&_ref);
#elif defined(ICE_HAS_ATOMIC_FUNCTIONS)
    assert(atomicExchangeAdd(0, &_ref) >= 0);
    atomicInc(&_ref);
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
#elif defined(ICE_HAS_ATOMIC_FUNCTIONS)
    assert(atomicExchangeAdd(0, &_ref) > 0);
    if(atomicDecAndTest(&_ref) && !_noDelete)
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
#elif defined(ICE_HAS_ATOMIC_FUNCTIONS)
    return atomicExchangeAdd(0, const_cast<AtomicCounter*>(&_ref));
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
