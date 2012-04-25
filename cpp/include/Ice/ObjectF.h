// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OBJECT_F_H
#define ICE_OBJECT_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

namespace Ice
{

class Object;
ICE_API IceUtil::Shared* upCast(::Ice::Object*);
typedef IceInternal::Handle< Object > ObjectPtr;

void ICE_API __patch__ObjectPtr(void*, ObjectPtr&);

}

#endif
