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

namespace IceInternal
{

class Connector;
void incRef(Connector*);
void decRef(Connector*);
typedef IceInternal::Handle<Connector> ConnectorPtr;

}

#endif
