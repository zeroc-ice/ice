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

class ConnectorI;
void ICE_API incRef(ConnectorI*);
void ICE_API decRef(ConnectorI*);
typedef __Ice::Handle<ConnectorI> Connector;

}

#endif
