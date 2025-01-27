// Copyright (c) ZeroC, Inc.

#ifndef ICE_COLLOCATED_REQUEST_HANDLER_H
#define ICE_COLLOCATED_REQUEST_HANDLER_H

#include "Ice/Logger.h"
#include "Ice/ObjectAdapterF.h"
#include "Ice/OutputStream.h"
#include "RequestHandler.h"
#include "TraceLevelsF.h"

#include <condition_variable>
#include <mutex>

namespace Ice
{
    class ObjectAdapterI;
    class OutgoingResponse;
}

namespace IceInternal
{
    class OutgoingAsyncBase;
    class OutgoingAsync;

    class CollocatedRequestHandler final : public RequestHandler,
                                           public std::enable_shared_from_this<CollocatedRequestHandler>
    {
    public:
        CollocatedRequestHandler(const ReferencePtr&, const Ice::ObjectAdapterPtr&);
        ~CollocatedRequestHandler() override;

        AsyncStatus sendAsyncRequest(const ProxyOutgoingAsyncBasePtr&) final;

        void asyncRequestCanceled(const OutgoingAsyncBasePtr&, std::exception_ptr) final;

        Ice::ConnectionIPtr getConnection() final;

        AsyncStatus invokeAsyncRequest(OutgoingAsyncBase*, int, bool);

        bool sentAsync(OutgoingAsyncBase*);

        void dispatchAll(Ice::InputStream&, std::int32_t, std::int32_t);

    private:
        void handleException(std::int32_t, std::exception_ptr);

        void sendResponse(Ice::OutgoingResponse);
        void dispatchException(std::int32_t, std::exception_ptr);

        const std::shared_ptr<Ice::ObjectAdapterI> _adapter;
        const bool _hasExecutor;
        const Ice::LoggerPtr _logger;
        const TraceLevelsPtr _traceLevels;

        int _requestId{0};
        std::map<OutgoingAsyncBasePtr, std::int32_t> _sendAsyncRequests;
        std::map<std::int32_t, OutgoingAsyncBasePtr> _asyncRequests;

        std::mutex _mutex;
    };
    using CollocatedRequestHandlerPtr = std::shared_ptr<CollocatedRequestHandler>;
}

#endif
