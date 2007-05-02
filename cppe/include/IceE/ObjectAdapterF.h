// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OBJECT_ADAPTER_F_H
#define ICEE_OBJECT_ADAPTER_F_H

#include <IceE/Handle.h>
#include <IceE/Shared.h>

namespace Ice
{

class ObjectAdapter;
inline bool operator==(const ObjectAdapter& l, const ObjectAdapter& r)
{
    return &l == &r;
}
inline bool operator<(const ObjectAdapter& l, const ObjectAdapter& r)
{
    return &l < &r;
}

}

namespace IceInternal
{

ICE_API IceUtil::Shared* upCast(::Ice::ObjectAdapter*);

}

namespace Ice
{

typedef ::IceInternal::Handle< ::Ice::ObjectAdapter> ObjectAdapterPtr;

}

#endif
