// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_CONNECT_REQUEST_HANDLER_H
#define ICE_CONNECT_REQUEST_HANDLER_H

#include <IceUtil/Monitor.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/UniquePtr.h>

#include <Ice/ConnectRequestHandlerF.h>
#include <Ice/RequestHandler.h>
#include <Ice/Reference.h>
#include <Ice/RouterInfo.h>
#include <Ice/ProxyF.h>
#include <Ice/BasicStream.h>

#include <deque>
#include <set>

namespace IceInternal
{

class ConnectRequestHandler : public RequestHandler,
                              public Reference::GetConnectionCallback,
                              public RouterInfo::AddProxyCallback,
                              public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    ConnectRequestHandler(const ReferencePtr&, const Ice::ObjectPrx&);
    virtual ~ConnectRequestHandler();

    RequestHandlerPtr connect(const Ice::ObjectPrx&);
    virtual RequestHandlerPtr update(const RequestHandlerPtr&, const RequestHandlerPtr&);

    virtual bool sendRequest(ProxyOutgoingBase*);
    virtual AsyncStatus sendAsyncRequest(const ProxyOutgoingAsyncBasePtr&);

    virtual void requestCanceled(OutgoingBase*, const Ice::LocalException&);
    virtual void asyncRequestCanceled(const OutgoingAsyncBasePtr&, const Ice::LocalException&);

    virtual Ice::ConnectionIPtr getConnection();
    virtual Ice::ConnectionIPtr waitForConnection();

    virtual void setConnection(const Ice::ConnectionIPtr&, bool);
    virtual void setException(const Ice::LocalException&);

    virtual void addedProxy();

private:

    bool initialized();
    void flushRequests();

    struct Request
    {
        Request() : out(0)
        {
        }

        ProxyOutgoingBase* out;
        ProxyOutgoingAsyncBasePtr outAsync;
    };

    Ice::ObjectPrx _proxy;
    std::set<Ice::ObjectPrx> _proxies;

    Ice::ConnectionIPtr _connection;
    bool _compress;
    IceUtil::UniquePtr<Ice::LocalException> _exception;
    bool _initialized;
    bool _flushing;

    std::deque<Request> _requests;

    RequestHandlerPtr _requestHandler;
};

}

#endif
