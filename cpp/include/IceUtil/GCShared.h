// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_GC_SHARED_H
#define ICE_UTIL_GC_SHARED_H

#include <IceUtil/Config.h>
#include <IceUtil/GCRecMutex.h>
#include <IceUtil/Shared.h>
#include <set>

namespace IceUtil
{

class GC;
class GCShared;

typedef std::set<GCShared*> GCObjectSet;
extern ICE_UTIL_API GCObjectSet gcObjects; // Set of pointers to all existing classes with class data members.

typedef std::multiset<GCShared*> GCObjectMultiSet;

class ICE_UTIL_API GCShared : public noncopyable
{
public:

    GCShared() :
#ifndef ICE_HAS_ATOMIC_FUNCTIONS
	_ref(0),
#endif
	_noDelete(false)
    {
#ifdef ICE_HAS_ATOMIC_FUNCTIONS
	ice_atomic_set(&_ref, 0);
#endif
    }

    virtual ~GCShared()
    {
    }

    virtual void __incRef(); // First derived class with class data members overrides this.
    virtual void __decRef(); // Ditto.

    int __getRef() const
    {
	gcRecMutex._m->lock();
	int ref = __getRefUnsafe();
	gcRecMutex._m->unlock();
	return ref;
    }

    void __setNoDelete(bool);

    void __incRefUnsafe()
    {
#if defined(_WIN32)
	InterlockedIncrement(&_ref);
#elif defined(ICE_HAS_ATOMIC_FUNCTIONS)
	ice_atomic_inc(&_ref);
#else
	++_ref;
#endif
    }

    void __decRefUnsafe()
    {
#if defined(_WIN32)
	InterlockedDecrement(&_ref);
#elif defined(ICE_HAS_ATOMIC_FUNCTIONS)
	ice_atomic_dec_and_test(&_ref);
#else
	--_ref;
#endif
    }

    int __getRefUnsafe() const
    {
#if defined(_WIN32)
	return InterlockedExchangeAdd(const_cast<LONG*>(&_ref), 0);
#elif defined(ICE_HAS_ATOMIC_FUNCTIONS)
	return ice_atomic_exchange_add(0, const_cast<ice_atomic_t*>(&_ref));
#else
	return _ref;
#endif
    }

    virtual void __gcReachable(GCObjectMultiSet&) const = 0;
    virtual void __gcClear() = 0;

protected:

    static void __addObject(GCObjectMultiSet& c, GCShared* p)
    {
	gcRecMutex._m->lock();
	if(p)
	{
	    c.insert(p);
	}
	gcRecMutex._m->unlock();
    }

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
