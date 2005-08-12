// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_TRANSCEIVER_F_H
#define ICE_TRANSCEIVER_F_H

#include <IceE/Handle.h>

namespace IceInternal
{

class Transceiver;
ICE_API void incRef(Transceiver*);
ICE_API void decRef(Transceiver*);
typedef Handle<Transceiver> TransceiverPtr;

}

#endif
