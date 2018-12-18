// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_BT_INSTANCE_F_H
#define ICE_BT_INSTANCE_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

namespace IceBT
{

class Instance;
IceUtil::Shared* upCast(Instance*);
typedef IceInternal::Handle<Instance> InstancePtr;

class EndpointI;
#ifdef ICE_CPP11_MAPPING
using EndpointIPtr = ::std::shared_ptr<EndpointI>;
#else
IceUtil::Shared* upCast(EndpointI*);
typedef IceInternal::Handle<EndpointI> EndpointIPtr;
#endif

class TransceiverI;
IceUtil::Shared* upCast(TransceiverI*);
typedef IceInternal::Handle<TransceiverI> TransceiverIPtr;

class AcceptorI;
IceUtil::Shared* upCast(AcceptorI*);
typedef IceInternal::Handle<AcceptorI> AcceptorIPtr;

}

#endif
