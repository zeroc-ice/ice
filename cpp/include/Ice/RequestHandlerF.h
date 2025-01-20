// Copyright (c) ZeroC, Inc.

#ifndef ICE_REQUEST_HANDLER_F_H
#define ICE_REQUEST_HANDLER_F_H

#include <memory>

namespace IceInternal
{
    class CancellationHandler;
    using CancellationHandlerPtr = std::shared_ptr<CancellationHandler>;

    class RequestHandler;
    using RequestHandlerPtr = std::shared_ptr<RequestHandler>;

    class RequestHandlerCache;
    using RequestHandlerCachePtr = std::shared_ptr<RequestHandlerCache>;
}

#endif
