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

class Endpoint;
void incRef(Endpoint*);
void decRef(Endpoint*);
typedef __Ice::Handle<Endpoint> Endpoint_ptr;

}

#endif
