//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ENDPOINT_I_F_H
#define ICE_ENDPOINT_I_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

namespace IceInternal
{

class EndpointI;
class TcpEndpointI;
class UdpEndpointI;
class WSEndpoint;
class EndpointI_connectors;

using EndpointIPtr = ::std::shared_ptr<EndpointI>;
using TcpEndpointIPtr = ::std::shared_ptr<TcpEndpointI>;
using UdpEndpointIPtr = ::std::shared_ptr<UdpEndpointI>;
using WSEndpointPtr = ::std::shared_ptr<WSEndpoint>;
using EndpointI_connectorsPtr = ::std::shared_ptr<EndpointI_connectors>;

}

#endif
