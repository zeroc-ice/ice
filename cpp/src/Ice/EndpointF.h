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

namespace IceInternal
{

class Endpoint;
void incRef(Endpoint*);
void decRef(Endpoint*);
typedef IceInternal::Handle<Endpoint> EndpointPtr;

}

#endif
