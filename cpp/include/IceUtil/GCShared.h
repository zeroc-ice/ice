// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_UTIL_GCShared_H
#define ICE_UTIL_GCShared_H

#include <IceUtil/GCRecMutex.h>
#include <set>

namespace IceUtil
{

class GC;
class GCShared;

typedef std::set<GCShared*> GCObjectSet;
extern ICE_UTIL_API GCObjectSet gcObjects; // Set of pointers to all existing classes.

typedef std::multiset<GCShared*> GCObjectMultiSet;

class ICE_UTIL_API GCShared : public noncopyable
{
public:

    GCShared();
    virtual ~GCShared();
    void __incRef();
    void __decRef();
    int __getRef() const;
    void __setNoDelete(bool);
    void __decRefUnsafe();
    virtual void __gcReachable(GCObjectMultiSet&) const = 0;
    virtual void __gcClear() = 0;

protected:

    static void __addObject(GCObjectMultiSet&, GCShared*);

private:

    int _ref;
    bool _noDelete;
    bool _adopted;

    friend class IceUtil::GC;
};

inline
GCShared::GCShared()
{
    gcRecMutex._m->lock();
    _ref = 0;
    _noDelete = false;
    _adopted = false;
    gcRecMutex._m->unlock();
}

inline
GCShared::~GCShared()
{
    gcRecMutex._m->lock();
    GCObjectSet::size_type num = gcObjects.erase(this);
    assert(num == 1);
    gcRecMutex._m->unlock();
}

inline void
GCShared::__incRef()
{
    gcRecMutex._m->lock();
    assert(_ref >= 0);
    if(!_adopted && _ref == 0)
    {
        _adopted = true;
	std::pair<GCObjectSet::iterator, bool> rc = gcObjects.insert(this);
	assert(rc.second);
    }
    ++_ref;
    gcRecMutex._m->unlock();
}

inline void
GCShared::__decRef()
{
    gcRecMutex._m->lock();
    bool doDelete = false;
    assert(_ref > 0);
    if(--_ref == 0)
    {
	doDelete = !_noDelete;
	_noDelete = true;
    }
    if(doDelete)
    {
	delete this;
    }
    gcRecMutex._m->unlock();
}

inline int
GCShared::__getRef() const
{
    gcRecMutex._m->lock();
    int ref = _ref;
    gcRecMutex._m->unlock();
    return ref;
}

inline void
GCShared::__setNoDelete(bool b)
{
    gcRecMutex._m->lock();
    _noDelete = b;
    gcRecMutex._m->unlock();
}

inline void
GCShared::__decRefUnsafe()
{
    --_ref;
}

inline void
GCShared::__addObject(GCObjectMultiSet& c, GCShared* p)
{
    gcRecMutex._m->lock();
    if(p)
    {
	c.insert(p);
    }
    gcRecMutex._m->unlock();
}

}

#endif
