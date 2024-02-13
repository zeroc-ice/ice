//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_REQUEST_HANDLER_CACHE_H
#define ICE_REQUEST_HANDLER_CACHE_H

#include "Ice/OperationMode.h"
#include "Ice/RequestHandler.h"
#include "Ice/ConnectionF.h"
#include <mutex>

namespace IceInternal
{

class RequestHandlerCache final
{
public:

    RequestHandlerCache(const ReferencePtr&);

    RequestHandlerPtr getRequestHandler();

    Ice::ConnectionPtr getCachedConnection();

    void clear(const RequestHandlerPtr& handler);

    int handleException(
        std::exception_ptr ex,
        const RequestHandlerPtr& handler,
        Ice::OperationMode mode,
        bool sent,
        int& cnt);

private:

    const ReferencePtr _reference;
    std::mutex _mutex;
    RequestHandlerPtr _handler; // The cached handler, set only when _reference->getCacheConnection() is true.
};

}

#endif
