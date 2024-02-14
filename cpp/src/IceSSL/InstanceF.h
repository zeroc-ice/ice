//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_INSTANCE_F_H
#define ICESSL_INSTANCE_F_H

#include <memory>

namespace IceSSL
{

class Instance;
using InstancePtr = std::shared_ptr<Instance>;

class EndpointI;
using EndpointIPtr = std::shared_ptr<EndpointI>;

class AcceptorI;
}

#endif
