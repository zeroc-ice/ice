// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_TRANSCEIVER_F_H
#define ICEE_TRANSCEIVER_F_H

#include <IceE/Handle.h>

namespace IceInternal
{

class Transceiver;
ICEE_API void incRef(Transceiver*);
ICEE_API void decRef(Transceiver*);
typedef Handle<Transceiver> TransceiverPtr;

}

#endif
