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

class Transceiver;
void incRef(Transceiver*);
void decRef(Transceiver*);
typedef __Ice::Handle<Transceiver> Transceiver_ptr;

}

#endif
