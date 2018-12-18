// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_IP_ENDPOINT_I_F_H
#define ICE_IP_ENDPOINT_I_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

namespace IceInternal
{

class IPEndpointI;

#ifdef ICE_CPP11_MAPPING
using IPEndpointIPtr = ::std::shared_ptr<IPEndpointI>;
#else
ICE_API IceUtil::Shared* upCast(IPEndpointI*);
typedef Handle<IPEndpointI> IPEndpointIPtr;
#endif

class EndpointHostResolver;
ICE_API IceUtil::Shared* upCast(EndpointHostResolver*);
typedef Handle<EndpointHostResolver> EndpointHostResolverPtr;
}

#endif
