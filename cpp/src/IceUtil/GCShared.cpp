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

#include <IceUtil/GCRecMutex.h>
#include <IceUtil/GCShared.h>

using namespace IceUtil;

IceUtil::GCObjectSet IceUtil::gcObjects;

IceUtil::GCShared::GCShared()
    : _ref(0), _noDelete(false)
{
}

IceUtil::GCShared::~GCShared()
{
    gcRecMutex._m->lock();
    gcObjects.erase(this);
    gcRecMutex._m->unlock();
}

void
IceUtil::GCShared::__incRef()
{
    gcRecMutex._m->lock();
    assert(_ref >= 0);
    if(_ref == 0)
    {
#ifdef NDEBUG // To avoid annoying warnings about variables that are not used...
	gcObjects.insert(this);
#else
	std::pair<GCObjectSet::iterator, bool> rc = gcObjects.insert(this);
	assert(rc.second);
#endif
    }
    ++_ref;
    gcRecMutex._m->unlock();
}

void
IceUtil::GCShared::__decRef()
{
    gcRecMutex._m->lock();
    bool doDelete = false;
    assert(_ref > 0);
    if(--_ref == 0)
    {
	doDelete = !_noDelete;
	_noDelete = true;
#ifdef NDEBUG // To avoid annoying warnings about variables that are not used...
	gcObjects.erase(this);
#else
	GCObjectSet::size_type num = gcObjects.erase(this);
	assert(num == 1);
#endif
    }
    gcRecMutex._m->unlock();

    if(doDelete)
    {
	delete this;
    }
}

int
IceUtil::GCShared::__getRef() const
{
    gcRecMutex._m->lock();
    int ref = _ref;
    gcRecMutex._m->unlock();
    return ref;
}

void
IceUtil::GCShared::__setNoDelete(bool b)
{
    gcRecMutex._m->lock();
    _noDelete = b;
    gcRecMutex._m->unlock();
}

void
IceUtil::GCShared::__decRefUnsafe()
{
    --_ref;
}

void
IceUtil::GCShared::__addObject(GCObjectMultiSet& c, GCShared* p)
{
    gcRecMutex._m->lock();
    if(p)
    {
	c.insert(p);
    }
    gcRecMutex._m->unlock();
}
