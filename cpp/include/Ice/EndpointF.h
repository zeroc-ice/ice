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
void incRef(EndpointI*);
void decRef(EndpointI*);
typedef __Ice::Handle<EndpointI> Endpoint;

}

#endif
