// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_TCP_CONNECTOR_F_H
#define ICE_TCP_CONNECTOR_F_H

#include <Ice/Handle.h>

namespace __Ice
{

class TcpConnectorI;
void ICE_API incRef(TcpConnectorI*);
void ICE_API decRef(TcpConnectorI*);
typedef __Ice::Handle<TcpConnectorI> TcpConnector;

}

#endif
