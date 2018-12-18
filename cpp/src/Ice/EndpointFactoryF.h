// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_ENDPOINT_FACTORY_F_H
#define ICE_ENDPOINT_FACTORY_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class EndpointFactory;
ICE_API IceUtil::Shared* upCast(EndpointFactory*);
typedef Handle<EndpointFactory> EndpointFactoryPtr;

}

#endif
