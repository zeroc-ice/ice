// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_INSTANCE_F_H
#define ICE_SSL_INSTANCE_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceSSL
{

class Instance;
IceUtil::Shared* upCast(Instance*);
typedef IceInternal::Handle<Instance> InstancePtr;

class EndpointI;
IceUtil::Shared* upCast(EndpointI*);
typedef IceInternal::Handle<EndpointI> EndpointIPtr;

class AcceptorI;
IceUtil::Shared* upCast(AcceptorI*);
typedef IceInternal::Handle<AcceptorI> AcceptorIPtr;

}

#endif
