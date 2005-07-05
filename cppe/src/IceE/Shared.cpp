// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Shared.h>

using namespace IceE;

IceE::SimpleShared::SimpleShared() :
    _ref(0),
    _noDelete(false)
{
}

IceE::SimpleShared::SimpleShared(const SimpleShared&) :
    _ref(0),
    _noDelete(false)
{
}

IceE::Shared::Shared() :
#ifndef ICEE_HAS_ATOMIC_FUNCTIONS
    _ref(0),
#endif
    _noDelete(false)
{
#ifdef ICEE_HAS_ATOMIC_FUNCTIONS
    ice_atomic_set(&_ref, 0);
#endif
}

IceE::Shared::Shared(const Shared&) :
#ifndef ICEE_HAS_ATOMIC_FUNCTIONS
    _ref(0),
#endif
    _noDelete(false)
{
#ifdef ICEE_HAS_ATOMIC_FUNCTIONS
    ice_atomic_set(&_ref, 0);
#endif
}

int
IceE::Shared::__getRef() const
{
#if defined(_WIN32)
    return InterlockedExchangeAdd(const_cast<LONG*>(&_ref), 0);
#elif defined(ICEE_HAS_ATOMIC_FUNCTIONS)
    return ice_atomic_exchange_add(0, const_cast<ice_atomic_t*>(&_ref));
#else
    _mutex.lock();
    int ref = _ref;
    _mutex.unlock();
    return ref;
#endif
}

void
IceE::Shared::__setNoDelete(bool b)
{
    _noDelete = b;
}
