// Copyright (c) ZeroC, Inc.

#ifndef ICE_CONNECT_REQUEST_HANDLER_H
#define ICE_CONNECT_REQUEST_HANDLER_H

#include "ConnectRequestHandlerF.h"
#include "Reference.h"
#include "RequestHandler.h"
#include "RouterInfo.h"

#include <condition_variable>
#include <deque>
#include <mutex>
#include <set>

namespace IceInternal
{
    class ConnectRequestHandler final : public RequestHandler,
                                        public std::enable_shared_from_this<ConnectRequestHandler>
    {
    public:
        ConnectRequestHandler(const ReferencePtr&);

        AsyncStatus sendAsyncRequest(const ProxyOutgoingAsyncBasePtr&) final;

        void asyncRequestCanceled(const OutgoingAsyncBasePtr&, std::exception_ptr) final;

        Ice::ConnectionIPtr getConnection() final;

        // setConnection and setException are the response and exception for RoutableReference::getConnectionAsync.
        void setConnection(Ice::ConnectionIPtr, bool);
        void setException(std::exception_ptr);

    private:
        bool initialized(std::unique_lock<std::mutex>&);
        void flushRequests();

        Ice::ConnectionIPtr _connection;
        bool _compress;
        std::exception_ptr _exception;
        bool _initialized{false};
        bool _flushing{false};

        std::deque<ProxyOutgoingAsyncBasePtr> _requests;

        std::mutex _mutex;
        std::condition_variable _conditionVariable;
    };
}

#endif
