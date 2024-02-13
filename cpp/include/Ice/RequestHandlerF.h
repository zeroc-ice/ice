//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_REQUEST_HANDLER_F_H
#define ICE_REQUEST_HANDLER_F_H

#include "Config.h"
#include <memory>

namespace IceInternal
{

class CancellationHandler;
class RequestHandler;
class RequestHandlerCache;

using CancellationHandlerPtr = ::std::shared_ptr<CancellationHandler>;
using RequestHandlerPtr = ::std::shared_ptr<RequestHandler>;
using RequestHandlerCachePtr = ::std::shared_ptr<RequestHandlerCache>;

}

#endif
