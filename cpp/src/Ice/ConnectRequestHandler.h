// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_CONNECT_REQUEST_HANDLER_H
#define ICE_CONNECT_REQUEST_HANDLER_H

#include <IceUtil/Monitor.h>
#include <IceUtil/Mutex.h>

#include <Ice/RequestHandler.h>
#include <Ice/Reference.h>
#include <Ice/RouterInfo.h>
#include <Ice/ProxyF.h>
#include <Ice/BasicStream.h>

#include <IceUtil/UniquePtr.h>
#include <deque>

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

    virtual RequestHandlerPtr connect();
    virtual RequestHandlerPtr update(const RequestHandlerPtr&, const RequestHandlerPtr&);

    virtual void prepareBatchRequest(BasicStream*);
    virtual void finishBatchRequest(BasicStream*);
    virtual void abortBatchRequest();

    virtual bool sendRequest(OutgoingBase*);
    virtual AsyncStatus sendAsyncRequest(const OutgoingAsyncBasePtr&);

    virtual void requestCanceled(OutgoingBase*, const Ice::LocalException&);
    virtual void asyncRequestCanceled(const OutgoingAsyncBasePtr&, const Ice::LocalException&);

    virtual Ice::ConnectionIPtr getConnection();
    virtual Ice::ConnectionIPtr waitForConnection();

    virtual void setConnection(const Ice::ConnectionIPtr&, bool);
    virtual void setException(const Ice::LocalException&);

    virtual void addedProxy();

    void flushRequestsWithException();

private:

    bool initialized();
    void flushRequests();

    struct Request
    {
        Request() : out(0), os(0)
        {
        }

        OutgoingBase* out;
        OutgoingAsyncBasePtr outAsync;
        BasicStream* os;
    };

    Ice::ObjectPrx _proxy;

    const bool _batchAutoFlush;

    Ice::ConnectionIPtr _connection;
    bool _compress;
    IceUtil::UniquePtr<Ice::LocalException> _exception;
    bool _initialized;
    bool _flushing;

    std::deque<Request> _requests;
    bool _batchRequestInProgress;
    size_t _batchRequestsSize;
    BasicStream _batchStream;
    bool _updateRequestHandler;
};
typedef IceUtil::Handle<ConnectRequestHandler> ConnectRequestHandlerPtr;

}

#endif
