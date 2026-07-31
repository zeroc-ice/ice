// Copyright (c) ZeroC, Inc.

#ifndef ICE_REQUEST_HANDLER_H
#define ICE_REQUEST_HANDLER_H

#include "Ice/ConnectionIF.h"
#include "Ice/OutgoingAsync.h"
#include "Ice/ReferenceF.h"
#include "Ice/RequestHandlerF.h"

namespace IceInternal
{
    //
    // An exception wrapper, which is used to notify that the request
    // handler should be cleared and the invocation retried.
    //
    class RetryException
    {
    public:
        RetryException(std::exception_ptr);
        RetryException(const RetryException&);

        [[nodiscard]] std::exception_ptr get() const;

    private:
        std::exception_ptr _ex;
    };

    class CancellationHandler
    {
    public:
        virtual ~CancellationHandler();

        virtual void asyncRequestCanceled(const OutgoingAsyncBasePtr&, std::exception_ptr) = 0;
    };

    class RequestHandler : public CancellationHandler
    {
    public:
        RequestHandler(const ReferencePtr&);
        ~RequestHandler() override;

        virtual AsyncStatus sendAsyncRequest(const ProxyOutgoingAsyncBasePtr&) = 0;

        // Returns this request handler's connection, or null if no connection is available. This function never
        // throws.
        virtual Ice::ConnectionIPtr getConnection() noexcept = 0;

    protected:
        const ReferencePtr _reference;
        const bool _response;
    };
}

#endif
