// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_ENDPOINT_FACTORY_MANAGER_F_H
#define ICE_ENDPOINT_FACTORY_MANAGER_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class EndpointFactoryManager;
void incRef(EndpointFactoryManager*);
void decRef(EndpointFactoryManager*);
typedef Handle<EndpointFactoryManager> EndpointFactoryManagerPtr;

}

#endif
