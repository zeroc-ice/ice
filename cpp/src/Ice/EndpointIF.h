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

#ifdef ICE_CPP11_MAPPING // C++11 mapping

using EndpointIPtr = ::std::shared_ptr<EndpointI>;
using TcpEndpointIPtr = ::std::shared_ptr<TcpEndpointI>;
using UdpEndpointIPtr = ::std::shared_ptr<UdpEndpointI>;
using WSEndpointPtr = ::std::shared_ptr<WSEndpoint>;
using EndpointI_connectorsPtr = ::std::shared_ptr<EndpointI_connectors>;

#else // C++98 mapping

ICE_API IceUtil::Shared* upCast(EndpointI*);
typedef Handle<EndpointI> EndpointIPtr;

ICE_API IceUtil::Shared* upCast(TcpEndpointI*);
typedef Handle<TcpEndpointI> TcpEndpointIPtr;

ICE_API IceUtil::Shared* upCast(UdpEndpointI*);
typedef Handle<UdpEndpointI> UdpEndpointIPtr;

ICE_API IceUtil::Shared* upCast(WSEndpoint*);
typedef Handle<WSEndpoint> WSEndpointPtr;

ICE_API IceUtil::Shared* upCast(EndpointI_connectors*);
typedef Handle<EndpointI_connectors> EndpointI_connectorsPtr;

#endif

}

#endif
