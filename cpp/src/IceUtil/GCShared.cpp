// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
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

void
IceUtil::GCShared::__incRef()
{
    gcRecMutex._m->lock();
    __incRefUnsafe();
    gcRecMutex._m->unlock();
}

void
IceUtil::GCShared::__decRef()
{
    gcRecMutex._m->lock();

    bool doDelete = false;
#if defined(_WIN32)
    if(InterlockedDecrement(&_ref) == 0)
#elif defined(ICE_HAS_ATOMIC_FUNCTIONS)
    if(ice_atomic_dec_and_test(&_ref))
#else
    if(--_ref == 0)
#endif
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

void
IceUtil::GCShared::__setNoDelete(bool b)
{
    gcRecMutex._m->lock();
    _noDelete = b;
    gcRecMutex._m->unlock();
}
