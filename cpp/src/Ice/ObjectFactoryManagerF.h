// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
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
typedef IceInternal::Handle<ObjectFactoryManager> ObjectFactoryManagerPtr;

}

#endif
