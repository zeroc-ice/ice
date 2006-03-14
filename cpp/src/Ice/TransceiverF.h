// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_TRANSCEIVER_F_H
#define ICE_TRANSCEIVER_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class Transceiver;
ICE_API void incRef(Transceiver*);
ICE_API void decRef(Transceiver*);
typedef Handle<Transceiver> TransceiverPtr;

}

#endif
