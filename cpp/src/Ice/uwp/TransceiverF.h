// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UWP_TRANSCEIVER_F_H
#define ICE_UWP_TRANSCEIVER_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

namespace IceInternal
{

class TcpEndpointI;
#ifndef  ICE_CPP11_MAPPING
ICE_API IceUtil::Shared* upCast(TcpEndpointI*);
#endif
ICE_DEFINE_PTR(TcpEndpointIPtr, TcpEndpointI);

class TcpAcceptor;
ICE_API IceUtil::Shared* upCast(TcpAcceptor*);
typedef IceInternal::Handle<TcpAcceptor> TcpAcceptorPtr;

}

#endif
