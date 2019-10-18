//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
ICE_API IceUtil::Shared* upCast(WSTransceiver*);
typedef Handle<WSTransceiver> WSTransceiverPtr;

}

#endif
