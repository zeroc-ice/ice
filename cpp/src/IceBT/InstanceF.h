//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
using EndpointIPtr = ::std::shared_ptr<EndpointI>;

class TransceiverI;
IceUtil::Shared* upCast(TransceiverI*);
typedef IceInternal::Handle<TransceiverI> TransceiverIPtr;

class AcceptorI;
IceUtil::Shared* upCast(AcceptorI*);
typedef IceInternal::Handle<AcceptorI> AcceptorIPtr;

}

#endif
