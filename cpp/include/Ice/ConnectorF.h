// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_CONNECTOR_F_H
#define ICE_CONNECTOR_F_H

#include <Ice/Handle.h>

namespace __Ice
{

class Connector;
void incRef(Connector*);
void decRef(Connector*);
typedef __Ice::Handle<Connector> Connector_ptr;

}

#endif
