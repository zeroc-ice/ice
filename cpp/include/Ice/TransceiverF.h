// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_TRANSCEIVER_F_H
#define ICE_TRANSCEIVER_F_H

#include <Ice/Handle.h>

namespace __Ice
{

class TransceiverI;
void incRef(TransceiverI*);
void decRef(TransceiverI*);
typedef __Ice::Handle<TransceiverI> Transceiver;

}

#endif
