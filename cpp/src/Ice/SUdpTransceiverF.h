// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SUDP_TRANSCEIVER_F_H
#define ICE_SUDP_TRANSCEIVER_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class SUdpTransceiver;
void incRef(SUdpTransceiver*);
void decRef(SUdpTransceiver*);
typedef Handle<SUdpTransceiver> SUdpTransceiverPtr;

}

#endif
