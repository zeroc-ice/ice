// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/GCShared.h>

namespace IceUtil
{

GCObjectSet gcObjects;

}

using namespace IceUtil;

IceUtil::GCShared::GCShared() :
    _ref(0),
    _noDelete(false)
{
}

IceUtil::GCShared::~GCShared()
{
}

void
IceUtil::GCShared::__incRef()
{
    gcRecMutex._m->lock();
    assert(_ref >= 0);
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
IceUtil::GCShared::__addObject(GCObjectMultiSet& c, GCShared* p)
{
    gcRecMutex._m->lock();
    if(p)
    {
	c.insert(p);
    }
    gcRecMutex._m->unlock();
}
