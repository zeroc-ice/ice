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

class ConnectRequestHandler final :
    public RequestHandler,
    public Reference::GetConnectionCallback,
    public std::enable_shared_from_this<ConnectRequestHandler>
{
public:

    ConnectRequestHandler(const ReferencePtr&, const Ice::ObjectPrxPtr&);

    RequestHandlerPtr connect(const Ice::ObjectPrxPtr&);
    virtual RequestHandlerPtr update(const RequestHandlerPtr&, const RequestHandlerPtr&);

    virtual AsyncStatus sendAsyncRequest(const ProxyOutgoingAsyncBasePtr&);

    virtual void asyncRequestCanceled(const OutgoingAsyncBasePtr&, std::exception_ptr);

    virtual Ice::ConnectionIPtr getConnection();
    virtual Ice::ConnectionIPtr waitForConnection();

    virtual void setConnection(const Ice::ConnectionIPtr&, bool);
    virtual void setException(std::exception_ptr);

    virtual void addedProxy();

private:

    bool initialized(std::unique_lock<std::mutex>&);
    void flushRequests();

    Ice::ObjectPrxPtr _proxy;
    std::set<Ice::ObjectPrxPtr> _proxies;

    Ice::ConnectionIPtr _connection;
    bool _compress;
    std::exception_ptr _exception;
    bool _initialized;
    bool _flushing;

    std::deque<ProxyOutgoingAsyncBasePtr> _requests;

    RequestHandlerPtr _requestHandler;
    std::mutex _mutex;
    std::condition_variable _conditionVariable;
};

}

#endif
