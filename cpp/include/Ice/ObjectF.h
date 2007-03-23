// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OBJECT_F_H
#define ICE_OBJECT_F_H

#include <Ice/Handle.h>
#include <Ice/GCCountMap.h>
#include <Ice/GCShared.h>

namespace Ice
{

class Object;

}

namespace IceInternal
{

ICE_API GCShared* upCast(::Ice::Object*);

}

namespace Ice
{

typedef IceInternal::Handle< Object > ObjectPtr;

void ICE_API __patch__ObjectPtr(void*, ObjectPtr&);

}

#endif
