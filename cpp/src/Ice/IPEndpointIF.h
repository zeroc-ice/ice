//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_IP_ENDPOINT_I_F_H
#define ICE_IP_ENDPOINT_I_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

namespace IceInternal
{

class IPEndpointI;

using IPEndpointIPtr = ::std::shared_ptr<IPEndpointI>;

class EndpointHostResolver;
ICE_API IceUtil::Shared* upCast(EndpointHostResolver*);
typedef Handle<EndpointHostResolver> EndpointHostResolverPtr;
}

#endif
