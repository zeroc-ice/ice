//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_COLLOCATED_REQUEST_HANDLER_H
#define ICE_COLLOCATED_REQUEST_HANDLER_H

#include <Ice/RequestHandler.h>
#include <Ice/OutputStream.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/LoggerF.h>
#include <Ice/TraceLevelsF.h>

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

    class CollocatedRequestHandler : public RequestHandler,
                                     public std::enable_shared_from_this<CollocatedRequestHandler>
    {
    public:
        CollocatedRequestHandler(const ReferencePtr&, const Ice::ObjectAdapterPtr&);
        virtual ~CollocatedRequestHandler();

        virtual AsyncStatus sendAsyncRequest(const ProxyOutgoingAsyncBasePtr&);

        virtual void asyncRequestCanceled(const OutgoingAsyncBasePtr&, std::exception_ptr);

        virtual Ice::ConnectionIPtr getConnection();
        virtual Ice::ConnectionIPtr waitForConnection();

        AsyncStatus invokeAsyncRequest(OutgoingAsyncBase*, int, bool);

        bool sentAsync(OutgoingAsyncBase*);

        void invokeAll(Ice::OutputStream*, std::int32_t, std::int32_t);

    private:
        void handleException(std::int32_t, std::exception_ptr);

        void sendResponse(Ice::OutgoingResponse);
        void sendResponse(std::int32_t, Ice::OutputStream*);
        void sendNoResponse();
        void invokeException(std::int32_t, std::exception_ptr);

        const std::shared_ptr<Ice::ObjectAdapterI> _adapter;
        const bool _hasExecutor;
        const Ice::LoggerPtr _logger;
        const TraceLevelsPtr _traceLevels;

        int _requestId;
        std::map<OutgoingAsyncBasePtr, std::int32_t> _sendAsyncRequests;
        std::map<std::int32_t, OutgoingAsyncBasePtr> _asyncRequests;

        std::mutex _mutex;
    };
    using CollocatedRequestHandlerPtr = std::shared_ptr<CollocatedRequestHandler>;
}

#endif
