// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_COLLOCATED_REQUEST_HANDLER_H
#define ICE_COLLOCATED_REQUEST_HANDLER_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>

#include <Ice/RequestHandler.h>
#include <Ice/ResponseHandler.h>
#include <Ice/BasicStream.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/LoggerF.h>
#include <Ice/TraceLevelsF.h>

namespace Ice
{

class ObjectAdapterI;
typedef IceUtil::Handle<ObjectAdapterI> ObjectAdapterIPtr;

}

namespace IceInternal
{

class OutgoingBase;
class Outgoing;
class OutgoingAsyncBase;
class OutgoingAsync;

class CollocatedRequestHandler : public RequestHandler, public ResponseHandler, private IceUtil::Monitor<IceUtil::Mutex>
{
public:

    CollocatedRequestHandler(const ReferencePtr&, const Ice::ObjectAdapterPtr&);
    virtual ~CollocatedRequestHandler();

    virtual RequestHandlerPtr update(const RequestHandlerPtr&, const RequestHandlerPtr&);

    virtual bool sendRequest(ProxyOutgoingBase*);
    virtual AsyncStatus sendAsyncRequest(const ProxyOutgoingAsyncBasePtr&);

    virtual void requestCanceled(OutgoingBase*, const Ice::LocalException&);
    virtual void asyncRequestCanceled(const OutgoingAsyncBasePtr&, const Ice::LocalException&);

    virtual void sendResponse(Ice::Int, BasicStream*, Ice::Byte, bool);
    virtual void sendNoResponse();
    virtual bool systemException(Ice::Int, const Ice::SystemException&, bool);
    virtual void invokeException(Ice::Int, const Ice::LocalException&, int, bool);

    const ReferencePtr& getReference() const { return _reference; } // Inlined for performances.

    virtual Ice::ConnectionIPtr getConnection();
    virtual Ice::ConnectionIPtr waitForConnection();

    void invokeRequest(OutgoingBase*, int);
    AsyncStatus invokeAsyncRequest(OutgoingAsyncBase*, int);

    bool sent(OutgoingBase*);
    bool sentAsync(OutgoingAsyncBase*);

    void invokeAll(BasicStream*, Ice::Int, Ice::Int);

private:

    void handleException(Ice::Int, const Ice::Exception&, bool);

    const Ice::ObjectAdapterIPtr _adapter;
    const bool _dispatcher;
    const Ice::LoggerPtr _logger;
    const TraceLevelsPtr _traceLevels;

    int _requestId;

    std::map<OutgoingBase*, Ice::Int> _sendRequests;
    std::map<OutgoingAsyncBasePtr, Ice::Int> _sendAsyncRequests;

    std::map<Ice::Int, OutgoingBase*> _requests;
    std::map<Ice::Int, OutgoingAsyncBasePtr> _asyncRequests;
};
typedef IceUtil::Handle<CollocatedRequestHandler> CollocatedRequestHandlerPtr;

}

#endif
