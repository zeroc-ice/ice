// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_TRANSCEIVER_F_H
#define ICE_TRANSCEIVER_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class Transceiver;
ICE_PROTOCOL_API void incRef(Transceiver*);
ICE_PROTOCOL_API void decRef(Transceiver*);
typedef Handle<Transceiver> TransceiverPtr;

}

#endif
