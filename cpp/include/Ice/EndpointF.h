// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_ENDPOINT_F_H
#define ICE_ENDPOINT_F_H

#include <Ice/Handle.h>

namespace __Ice
{

class EndpointI;
void ICE_API incRef(EndpointI*);
void ICE_API decRef(EndpointI*);
typedef __Ice::Handle<EndpointI> Endpoint;

class DgEndpointI;
void ICE_API incRef(DgEndpointI*);
void ICE_API decRef(DgEndpointI*);
typedef __Ice::Handle<DgEndpointI> DgEndpoint;

}

#endif
