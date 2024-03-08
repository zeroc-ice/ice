//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_COLLOCATED_REQUEST_HANDLER_H
#define ICE_COLLOCATED_REQUEST_HANDLER_H

#include <Ice/RequestHandler.h>
#include <Ice/ResponseHandler.h>
#include <Ice/OutputStream.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/LoggerF.h>
#include <Ice/TraceLevelsF.h>

#include <condition_variable>
#include <mutex>

namespace Ice
{
    class ObjectAdapterI;
}

namespace IceInternal
{
    class OutgoingAsyncBase;
    class OutgoingAsync;

    class CollocatedRequestHandler : public RequestHandler, public ResponseHandler
    {
    public:
        CollocatedRequestHandler(const ReferencePtr&, const Ice::ObjectAdapterPtr&);
        virtual ~CollocatedRequestHandler();

        virtual AsyncStatus sendAsyncRequest(const ProxyOutgoingAsyncBasePtr&);

        virtual void asyncRequestCanceled(const OutgoingAsyncBasePtr&, std::exception_ptr);

        virtual void sendResponse(std::int32_t, Ice::OutputStream*, std::uint8_t, bool);
        virtual void sendNoResponse();
        virtual bool systemException(std::int32_t, std::exception_ptr, bool);
        virtual void invokeException(std::int32_t, std::exception_ptr, int, bool);

        virtual Ice::ConnectionIPtr getConnection();
        virtual Ice::ConnectionIPtr waitForConnection();

        AsyncStatus invokeAsyncRequest(OutgoingAsyncBase*, int, bool);

        bool sentAsync(OutgoingAsyncBase*);

        void invokeAll(Ice::OutputStream*, std::int32_t, std::int32_t);

        std::shared_ptr<CollocatedRequestHandler> shared_from_this()
        {
            return std::static_pointer_cast<CollocatedRequestHandler>(ResponseHandler::shared_from_this());
        }

    private:
        void handleException(std::int32_t, std::exception_ptr, bool);

        const std::shared_ptr<Ice::ObjectAdapterI> _adapter;
        const bool _dispatcher;
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
