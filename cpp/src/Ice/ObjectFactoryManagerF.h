// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SERVANT_FACTORY_MANAGER_F_H
#define ICE_SERVANT_FACTORY_MANAGER_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class ObjectFactoryManager;
void incRef(ObjectFactoryManager*);
void decRef(ObjectFactoryManager*);
typedef Handle<ObjectFactoryManager> ObjectFactoryManagerPtr;

}

#endif
