// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_IP_ENDPOINT_I_F_H
#define ICE_IP_ENDPOINT_I_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

namespace IceInternal
{

class IPEndpointI;
ICE_API IceUtil::Shared* upCast(IPEndpointI*);
typedef Handle<IPEndpointI> IPEndpointIPtr;

class EndpointHostResolver;
ICE_API IceUtil::Shared* upCast(EndpointHostResolver*);
typedef Handle<EndpointHostResolver> EndpointHostResolverPtr;

}

#endif
