// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_TCP_TRANSCEIVER_F_H
#define ICE_TCP_TRANSCEIVER_F_H

#include <Ice/Handle.h>

namespace __Ice
{

class TcpTransceiverI;
void ICE_API incRef(TcpTransceiverI*);
void ICE_API decRef(TcpTransceiverI*);
typedef __Ice::Handle<TcpTransceiverI> TcpTransceiver;

}

#endif
