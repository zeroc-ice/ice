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

namespace _Ice
{

void ICE_API _incRef(::Ice::EndpointI*);
void ICE_API _decRef(::Ice::EndpointI*);

}

namespace Ice
{

typedef _Ice::Handle<EndpointI> Endpoint;

}

#endif
