//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_REQUEST_HANDLER_F_H
#define ICE_REQUEST_HANDLER_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

namespace IceInternal
{

class CancellationHandler;
class RequestHandler;

using CancellationHandlerPtr = ::std::shared_ptr<CancellationHandler>;
using RequestHandlerPtr = ::std::shared_ptr<RequestHandler>;

}

#endif
