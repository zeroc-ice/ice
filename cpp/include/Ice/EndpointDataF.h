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

namespace _Ice
{

class EndpointDataI;
void ICE_API _incRef(EndpointDataI*);
void ICE_API _decRef(EndpointDataI*);
typedef _Ice::Handle<EndpointDataI> EndpointData;

}

#endif
