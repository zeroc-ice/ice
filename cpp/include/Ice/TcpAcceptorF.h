// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_TCP_ACCEPTOR_F_H
#define ICE_TCP_ACCEPTOR_F_H

#include <Ice/Handle.h>

namespace __Ice
{

class TcpAcceptorI;
void ICE_API incRef(TcpAcceptorI*);
void ICE_API decRef(TcpAcceptorI*);
typedef __Ice::Handle<TcpAcceptorI> TcpAcceptor;

}

#endif
