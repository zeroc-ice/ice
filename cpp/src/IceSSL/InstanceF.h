// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_INSTANCE_F_H
#define ICESSL_INSTANCE_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>
#include <IceSSL/Plugin.h>

namespace IceSSL
{

class Instance;
ICESSL_API IceUtil::Shared* upCast(Instance*);
typedef IceInternal::Handle<Instance> InstancePtr;

class EndpointI;
#ifdef ICE_CPP11_MAPPING
typedef ::std::shared_ptr<EndpointI> EndpointIPtr;
#else
ICESSL_API IceUtil::Shared* upCast(EndpointI*);
typedef IceInternal::Handle<EndpointI> EndpointIPtr;
#endif

class AcceptorI;
ICESSL_API IceUtil::Shared* upCast(AcceptorI*);
typedef IceInternal::Handle<AcceptorI> AcceptorIPtr;

}

#endif
