// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_CONNECT_REQUEST_HANDLER_H
#define ICE_CONNECT_REQUEST_HANDLER_H

#include <IceUtil/Monitor.h>
#include <IceUtil/Mutex.h>
#include <Ice/UniquePtr.h>

#include <Ice/ConnectRequestHandlerF.h>
#include <Ice/RequestHandler.h>
#include <Ice/Reference.h>
#include <Ice/RouterInfo.h>
#include <Ice/ProxyF.h>

#include <deque>
#include <set>

namespace IceInternal
{

class ConnectRequestHandler ICE_FINAL : public RequestHandler,
                                        public Reference::GetConnectionCallback,
                                        public RouterInfo::AddProxyCallback,
                                        public IceUtil::Monitor<IceUtil::Mutex>
#ifdef ICE_CPP11_MAPPING
                            , public std::enable_shared_from_this<ConnectRequestHandler>
#endif
{
public:

    ConnectRequestHandler(const ReferencePtr&, const Ice::ObjectPrxPtr&);

    RequestHandlerPtr connect(const Ice::ObjectPrxPtr&);
    virtual RequestHandlerPtr update(const RequestHandlerPtr&, const RequestHandlerPtr&);

    virtual AsyncStatus sendAsyncRequest(const ProxyOutgoingAsyncBasePtr&);

    virtual void asyncRequestCanceled(const OutgoingAsyncBasePtr&, const Ice::LocalException&);

    virtual Ice::ConnectionIPtr getConnection();
    virtual Ice::ConnectionIPtr waitForConnection();

    virtual void setConnection(const Ice::ConnectionIPtr&, bool);
    virtual void setException(const Ice::LocalException&);

    virtual void addedProxy();

private:

    bool initialized();
    void flushRequests();

    Ice::ObjectPrxPtr _proxy;
    std::set<Ice::ObjectPrxPtr> _proxies;

    Ice::ConnectionIPtr _connection;
    bool _compress;
    IceInternal::UniquePtr<Ice::LocalException> _exception;
    bool _initialized;
    bool _flushing;

    std::deque<ProxyOutgoingAsyncBasePtr> _requests;

    RequestHandlerPtr _requestHandler;
};

}

#endif
