// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/GCShared.h>

namespace IceInternal
{

GCObjectSet gcObjects;

}

using namespace IceInternal;

IceInternal::GCShared::GCShared() :
    _ref(0),
    _noDelete(false)
{
}

IceInternal::GCShared::GCShared(const GCShared&) :
    _ref(0),
    _noDelete(false)
{
}

void
IceInternal::GCShared::__incRef()
{
    gcRecMutex._m->lock();
    assert(_ref >= 0);
    ++_ref;
    gcRecMutex._m->unlock();
}

void
IceInternal::GCShared::__decRef()
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
IceInternal::GCShared::__getRef() const
{
    gcRecMutex._m->lock();
    int ref = _ref;
    gcRecMutex._m->unlock();
    return ref;
}

void
IceInternal::GCShared::__setNoDelete(bool b)
{
    gcRecMutex._m->lock();
    _noDelete = b;
    gcRecMutex._m->unlock();
}
