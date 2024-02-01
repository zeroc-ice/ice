//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
typedef ::std::shared_ptr<EndpointI> EndpointIPtr;

class AcceptorI;
ICESSL_API IceUtil::Shared* upCast(AcceptorI*);
typedef IceInternal::Handle<AcceptorI> AcceptorIPtr;

}

#endif
