// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ENDPOINT_I_F_H
#define ICE_ENDPOINT_I_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

namespace IceInternal
{

class EndpointI;
ICE_API IceUtil::Shared* upCast(EndpointI*);
typedef Handle<EndpointI> EndpointIPtr;

class EndpointI_connectors;
ICE_API IceUtil::Shared* upCast(EndpointI_connectors*);
typedef Handle<EndpointI_connectors> EndpointI_connectorsPtr;

}

#endif
