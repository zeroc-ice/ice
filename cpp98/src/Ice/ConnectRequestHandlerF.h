//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_CONNECT_REQUEST_HANDLER_F_H
#define ICE_CONNECT_REQUEST_HANDLER_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

namespace IceInternal
{

class ConnectRequestHandler;
#ifdef ICE_CPP11_MAPPING
using ConnectRequestHandlerPtr = ::std::shared_ptr<ConnectRequestHandler>;
#else
IceUtil::Shared* upCast(ConnectRequestHandler*);
typedef IceInternal::Handle<ConnectRequestHandler> ConnectRequestHandlerPtr;
#endif

}

#endif
