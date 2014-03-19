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

    ConnectRequestHandler(const ReferencePtr&, const Ice::ObjectPrx&, const Handle< ::IceDelegate::Ice::Object>&);
    virtual ~ConnectRequestHandler();

    RequestHandlerPtr connect();

    virtual void prepareBatchRequest(BasicStream*);
    virtual void finishBatchRequest(BasicStream*);
    virtual void abortBatchRequest();

    virtual Ice::ConnectionI* sendRequest(Outgoing*);
    virtual AsyncStatus sendAsyncRequest(const OutgoingAsyncPtr&);

    virtual bool flushBatchRequests(BatchOutgoing*);
    virtual AsyncStatus flushAsyncBatchRequests(const BatchOutgoingAsyncPtr&);

    virtual Ice::ConnectionIPtr getConnection(bool);

    virtual void setConnection(const Ice::ConnectionIPtr&, bool);
    virtual void setException(const Ice::LocalException&);

    virtual void addedProxy();

    void flushRequestsWithException(const Ice::LocalException&);
    void flushRequestsWithException(const LocalExceptionWrapper&);

private:

    bool initialized();
    void flushRequests();

    struct Request
    {
        OutgoingAsyncPtr out;
        BatchOutgoingAsyncPtr batchOut;
        BasicStream* os;
    };

    Ice::ObjectPrx _proxy;
    Handle< ::IceDelegate::Ice::Object> _delegate;

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
