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

namespace _Ice
{

class TransceiverI;
void ICE_API _incRef(TransceiverI*);
void ICE_API _decRef(TransceiverI*);
typedef _Ice::Handle<TransceiverI> Transceiver;

}

#endif
