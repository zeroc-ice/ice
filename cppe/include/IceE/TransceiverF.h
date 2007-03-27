// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_TRANSCEIVER_F_H
#define ICEE_TRANSCEIVER_F_H

#include <IceE/Handle.h>
#include <IceE/Shared.h>

namespace IceInternal
{

class Transceiver;
ICE_API IceUtil::Shared* upCast(Transceiver*);
typedef Handle<Transceiver> TransceiverPtr;

}

#endif
