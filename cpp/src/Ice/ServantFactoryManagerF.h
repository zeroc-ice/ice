// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_VALUE_FACTORY_MANAGER_F_H
#define ICE_VALUE_FACTORY_MANAGER_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class ServantFactoryManager;
void incRef(ServantFactoryManager*);
void decRef(ServantFactoryManager*);
typedef IceInternal::Handle<ServantFactoryManager> ServantFactoryManagerPtr;

}

#endif
