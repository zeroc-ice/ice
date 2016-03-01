// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ENDPOINT_FACTORY_MANAGER_F_H
#define ICE_ENDPOINT_FACTORY_MANAGER_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class EndpointFactoryManager;
IceUtil::Shared* upCast(EndpointFactoryManager*);
typedef Handle<EndpointFactoryManager> EndpointFactoryManagerPtr;

}

#endif
