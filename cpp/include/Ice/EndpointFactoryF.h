// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_ENDPOINT_FACTORY_F_H
#define ICE_ENDPOINT_FACTORY_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class EndpointFactory;
ICE_PROTOCOL_API void incRef(EndpointFactory*);
ICE_PROTOCOL_API void decRef(EndpointFactory*);
typedef Handle<EndpointFactory> EndpointFactoryPtr;

}

#endif
