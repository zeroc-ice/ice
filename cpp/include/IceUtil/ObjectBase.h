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

#ifndef ICE_UTIL_OBJECTBASE_H
#define ICE_UTIL_OBJECTBASE_H

#include <IceUtil/StaticRecMutex.h>
#include <set>

namespace IceUtil
{

extern ICE_UTIL_API StaticRecMutex gcMutex;

class GC;
class ObjectBase;

typedef std::set<ObjectBase*> ObjectSet;
extern ICE_UTIL_API ObjectSet objects; // Set of pointers to all existing classes.

typedef std::multiset<ObjectBase*> ObjectMultiSet;

class ICE_UTIL_API ObjectBase : public noncopyable
{
public:

    ObjectBase();
    virtual ~ObjectBase();
    void __incRef();
    void __decRef();
    int __getRef() const;
    void __setNoDelete(bool);
    void __decRefUnsafe();

protected:

    static void __addObject(ObjectMultiSet&, ObjectBase*);
    virtual void __gcReachable(ObjectMultiSet&) const = 0;
    virtual void __gcClear() = 0;

private:

    int _ref;
    bool _noDelete;
    bool _adopted;

    friend class IceUtil::GC;
};

inline
ObjectBase::ObjectBase()
{
    gcMutex.lock();
    _ref = 0;
    _noDelete = false;
    _adopted = false;
    gcMutex.unlock();
}

inline
ObjectBase::~ObjectBase()
{
    gcMutex.lock();
    ObjectSet::size_type num = objects.erase(this);
    assert(num == 1);
    gcMutex.unlock();
}

inline void
ObjectBase::__incRef()
{
    gcMutex.lock();
    assert(_ref >= 0);
    if(!_adopted && _ref == 0)
    {
        _adopted = true;
	std::pair<ObjectSet::iterator, bool> rc = objects.insert(this);
	assert(rc.second);
    }
    ++_ref;
    gcMutex.unlock();
}

inline void
ObjectBase::__decRef()
{
    gcMutex.lock();
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
    gcMutex.unlock();
}

inline int
ObjectBase::__getRef() const
{
    gcMutex.lock();
    int ref = _ref;
    gcMutex.unlock();
    return ref;
}

inline void
ObjectBase::__setNoDelete(bool b)
{
    gcMutex.lock();
    _noDelete = b;
    gcMutex.unlock();
}

inline void
ObjectBase::__decRefUnsafe()
{
    --_ref;
}

inline void
ObjectBase::__addObject(ObjectMultiSet& c, ObjectBase* p)
{
    gcMutex.lock();
    if(p)
    {
	c.insert(p);
    }
    gcMutex.unlock();
}

}

#endif
