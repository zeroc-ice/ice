// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
