// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Shared.h>

using namespace IceUtil;

//
// Flag constant used by the Shared class. Derived classes
// such as GCObject define more flag constants.
//
const unsigned char IceUtil::Shared::NoDelete = 1;

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
    _flags(0)
{
}

IceUtil::Shared::Shared(const Shared&) :
    _ref(0),
    _flags(0)
{
}

void 
IceUtil::Shared::__incRef()
{
#if defined(ICE_CPP11)
    assert(_ref.fetch_add(0, std::memory_order_relaxed) >= 0);
    ++_ref;
#elif defined(_WIN32)
    assert(InterlockedExchangeAdd(&_ref, 0) >= 0);
    InterlockedIncrement(&_ref);
#elif defined(ICE_HAS_GCC_BUILTINS)

#   ifndef NDEBUG
    int c = 
#   endif
        __sync_fetch_and_add(&_ref, 1);
    assert(c >= 0);
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
#if defined(ICE_CPP11)
    assert(_ref.fetch_add(0, std::memory_order_relaxed) > 0);
    if(--_ref == 0 && !(_flags & NoDelete))
    {
        delete this;
    }
#elif defined(_WIN32)
    assert(InterlockedExchangeAdd(&_ref, 0) > 0);
    if(InterlockedDecrement(&_ref) == 0 && !(_flags & NoDelete))
    {
        delete this;
    }
#elif defined(ICE_HAS_GCC_BUILTINS)
    int c = __sync_fetch_and_sub(&_ref, 1);
    assert(c > 0);
    if(c == 1 && !(_flags & NoDelete))
    {
        delete this;
    }
#else
    bool doDelete = false;
    _mutex.lock();
    assert(_ref > 0);
    if(--_ref == 0)
    {
        doDelete = !(_flags & NoDelete);
        _flags |= NoDelete;
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
#if defined(ICE_CPP11)
    return _ref.load(std::memory_order_relaxed);
#elif defined(_WIN32)
    return InterlockedExchangeAdd(const_cast<LONG*>(&_ref), 0);
#elif defined(ICE_HAS_GCC_BUILTINS)
    return __sync_fetch_and_sub(const_cast<volatile int*>(&_ref), 0);
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
    _flags = b ? (_flags | NoDelete) : (_flags & ~NoDelete);
}
