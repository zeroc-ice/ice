//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_CONNECT_REQUEST_HANDLER_H
#define ICE_CONNECT_REQUEST_HANDLER_H

#include <Ice/ConnectRequestHandlerF.h>
#include <Ice/RequestHandler.h>
#include <Ice/Reference.h>
#include <Ice/RouterInfo.h>
#include <Ice/ProxyF.h>

#include <condition_variable>
#include <deque>
#include <mutex>
#include <set>

namespace IceInternal
{

class ConnectRequestHandler final : public RequestHandler, public std::enable_shared_from_this<ConnectRequestHandler>
{
public:

    ConnectRequestHandler(const ReferencePtr&);

    RequestHandlerPtr connect();

    virtual AsyncStatus sendAsyncRequest(const ProxyOutgoingAsyncBasePtr&);

    virtual void asyncRequestCanceled(const OutgoingAsyncBasePtr&, std::exception_ptr);

    virtual Ice::ConnectionIPtr getConnection();
    virtual Ice::ConnectionIPtr waitForConnection();

    // setConnection and setException are the response and exception for RoutableReference::getConnectionAsync.
    void setConnection(Ice::ConnectionIPtr, bool);
    void setException(std::exception_ptr);

private:

    bool initialized(std::unique_lock<std::mutex>&);
    void flushRequests();

    Ice::ConnectionIPtr _connection;
    bool _compress;
    std::exception_ptr _exception;
    bool _initialized;
    bool _flushing;

    std::deque<ProxyOutgoingAsyncBasePtr> _requests;

    std::mutex _mutex;
    std::condition_variable _conditionVariable;
};

}

#endif
