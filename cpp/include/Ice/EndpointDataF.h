// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_ENDPOINT_DATA_F_H
#define ICE_ENDPOINT_DATA_F_H

#include <Ice/Handle.h>

namespace __Ice
{

class EndpointDataI;
void ICE_API incRef(EndpointDataI*);
void ICE_API decRef(EndpointDataI*);
typedef __Ice::Handle<EndpointDataI> EndpointData;

}

#endif
