// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OBJECT_F_H
#define ICEE_OBJECT_F_H

#include <IceE/Handle.h>

namespace Ice
{

class Object;

}

namespace IceInternal
{

ICEE_API void incRef(::Ice::Object*);
ICEE_API void decRef(::Ice::Object*);

}

namespace Ice
{

typedef IceInternal::Handle< Object > ObjectPtr;

}

#endif
