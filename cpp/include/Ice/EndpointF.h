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

class TcpEndpointI;
void ICE_API incRef(TcpEndpointI*);
void ICE_API decRef(TcpEndpointI*);
typedef __Ice::Handle<TcpEndpointI> TcpEndpoint;

class SslEndpointI;
void ICE_API incRef(SslEndpointI*);
void ICE_API decRef(SslEndpointI*);
typedef __Ice::Handle<SslEndpointI> SslEndpoint;

class UdpEndpointI;
void ICE_API incRef(UdpEndpointI*);
void ICE_API decRef(UdpEndpointI*);
typedef __Ice::Handle<UdpEndpointI> UdpEndpoint;

}

#endif
