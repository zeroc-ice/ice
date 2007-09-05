// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Shared.h>

using namespace IceUtil;

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
    ice_atomic_set(&_ref, 0);
#endif
}

IceUtil::Shared::Shared(const Shared&) :
#ifndef ICE_HAS_ATOMIC_FUNCTIONS
    _ref(0),
#endif
    _noDelete(false)
{
#ifdef ICE_HAS_ATOMIC_FUNCTIONS
    ice_atomic_set(&_ref, 0);
#endif
}

void 
IceUtil::Shared::__incRef()
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

int
IceUtil::Shared::__getRef() const
{
#if defined(_WIN32)
    return InterlockedExchangeAdd(const_cast<LONG*>(&_ref), 0);
#elif defined(ICE_HAS_ATOMIC_FUNCTIONS)
    return ice_atomic_exchange_add(0, const_cast<ice_atomic_t*>(&_ref));
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
