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

namespace IceSSL
{

class EndpointI;
#ifndef  ICE_CPP11_MAPPING
ICE_API IceUtil::Shared* upCast(EndpointI*);
#endif
ICE_DEFINE_PTR(EndpointIPtr, EndpointI);

class AcceptorI;
ICE_API IceUtil::Shared* upCast(AcceptorI*);
typedef IceInternal::Handle<AcceptorI> AcceptorIPtr;

}

#endif
