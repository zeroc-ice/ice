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

namespace _Ice
{

class ConnectorI;
void ICE_API _incRef(ConnectorI*);
void ICE_API _decRef(ConnectorI*);
typedef _Ice::Handle<ConnectorI> Connector;

}

#endif
