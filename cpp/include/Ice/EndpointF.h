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

namespace Ice { class EndpointI; }

namespace __Ice
{

void ICE_API incRef(::Ice::EndpointI*);
void ICE_API decRef(::Ice::EndpointI*);

}

namespace Ice
{

typedef __Ice::Handle<EndpointI> Endpoint;

}

#endif
