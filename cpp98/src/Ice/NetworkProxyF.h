//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_NETWORK_PROXY_F_H
#define ICE_NETWORK_PROXY_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class NetworkProxy;
ICE_API IceUtil::Shared* upCast(NetworkProxy*);
typedef Handle<NetworkProxy> NetworkProxyPtr;

}

#endif
