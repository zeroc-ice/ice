// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BT_ENGINE_F_H
#define ICE_BT_ENGINE_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

namespace IceBT
{

class Engine;
IceUtil::Shared* upCast(Engine*);
typedef IceInternal::Handle<Engine> EnginePtr;

class ManagedObjects;
IceUtil::Shared* upCast(ManagedObjects*);
typedef IceInternal::Handle<ManagedObjects> ManagedObjectsPtr;

}

#endif
