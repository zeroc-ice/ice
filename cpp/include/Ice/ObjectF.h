// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
ICE_API Object* upCast(Object*);
typedef IceInternal::Handle< Object > ObjectPtr;
ICE_API void __patch(ObjectPtr&, const ObjectPtr&);

}

#endif
