// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
