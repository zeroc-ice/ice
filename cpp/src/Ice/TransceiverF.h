// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_TRANSCEIVER_F_H
#define ICE_TRANSCEIVER_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class Transceiver;
ICE_API IceUtil::Shared* upCast(Transceiver*);
typedef Handle<Transceiver> TransceiverPtr;

class TcpTransceiver;
ICE_API IceUtil::Shared* upCast(TcpTransceiver*);
typedef Handle<TcpTransceiver> TcpTransceiverPtr;

class UdpTransceiver;
ICE_API IceUtil::Shared* upCast(UdpTransceiver*);
typedef Handle<UdpTransceiver> UdpTransceiverPtr;

class WSTransceiver;
ICE_API IceUtil::Shared* upCast(Transceiver*);
typedef Handle<Transceiver> TransceiverPtr;

}

#endif
