// Copyright (c) ZeroC, Inc.

#ifndef ICE_REQUEST_HANDLER_CACHE_H
#define ICE_REQUEST_HANDLER_CACHE_H

#include "Ice/ConnectionF.h"
#include "RequestHandler.h"
#include <mutex>

namespace IceInternal
{
    // Represents a holder/cache for a request handler. It's tied to a single Reference, and can be shared by multiple
    // proxies (all with the same Reference).
    class RequestHandlerCache final
    {
    public:
        RequestHandlerCache(const ReferencePtr&);

        RequestHandlerPtr getRequestHandler();

        Ice::ConnectionPtr getCachedConnection();

        void clearCachedRequestHandler(const RequestHandlerPtr& handler);

    private:
        const ReferencePtr _reference;
        const bool _cacheConnection;
        std::mutex _mutex;                       // protects _cachedRequestHandler
        RequestHandlerPtr _cachedRequestHandler; // set only when _cacheConnection is true.
    };
}

#endif
