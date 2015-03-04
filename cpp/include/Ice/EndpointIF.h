// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ENDPOINT_I_F_H
#define ICE_ENDPOINT_I_F_H

#include <Ice/Handle.h>
#include <Ice/LocalObject.h>

namespace IceInternal
{

class EndpointI;
ICE_API Ice::LocalObject* upCast(EndpointI*);
typedef Handle<EndpointI> EndpointIPtr;

class EndpointHostResolver;
ICE_API IceUtil::Shared* upCast(EndpointHostResolver*);
typedef Handle<EndpointHostResolver> EndpointHostResolverPtr;

}

#endif
