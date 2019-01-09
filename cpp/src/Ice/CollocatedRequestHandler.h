// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_COLLOCATED_REQUEST_HANDLER_H
#define ICE_COLLOCATED_REQUEST_HANDLER_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>

#include <Ice/RequestHandler.h>
#include <Ice/ResponseHandler.h>
#include <Ice/OutputStream.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/LoggerF.h>
#include <Ice/TraceLevelsF.h>

namespace Ice
{

class ObjectAdapterI;
ICE_DEFINE_PTR(ObjectAdapterIPtr, ObjectAdapterI);

}

namespace IceInternal
{

class OutgoingAsyncBase;
class OutgoingAsync;

class CollocatedRequestHandler : public RequestHandler,
                                 public ResponseHandler,
                                 private IceUtil::Monitor<IceUtil::Mutex>
{
public:

    CollocatedRequestHandler(const ReferencePtr&, const Ice::ObjectAdapterPtr&);
    virtual ~CollocatedRequestHandler();

    virtual RequestHandlerPtr update(const RequestHandlerPtr&, const RequestHandlerPtr&);

    virtual AsyncStatus sendAsyncRequest(const ProxyOutgoingAsyncBasePtr&);

    virtual void asyncRequestCanceled(const OutgoingAsyncBasePtr&, const Ice::LocalException&);

    virtual void sendResponse(Ice::Int, Ice::OutputStream*, Ice::Byte, bool);
    virtual void sendNoResponse();
    virtual bool systemException(Ice::Int, const Ice::SystemException&, bool);
    virtual void invokeException(Ice::Int, const Ice::LocalException&, int, bool);

    const ReferencePtr& getReference() const { return _reference; } // Inlined for performances.

    virtual Ice::ConnectionIPtr getConnection();
    virtual Ice::ConnectionIPtr waitForConnection();

    AsyncStatus invokeAsyncRequest(OutgoingAsyncBase*, int, bool);

    bool sentAsync(OutgoingAsyncBase*);

    void invokeAll(Ice::OutputStream*, Ice::Int, Ice::Int);

#ifdef ICE_CPP11_MAPPING
    std::shared_ptr<CollocatedRequestHandler> shared_from_this()
    {
        return std::static_pointer_cast<CollocatedRequestHandler>(ResponseHandler::shared_from_this());
    }
#endif

private:

    void handleException(Ice::Int, const Ice::Exception&, bool);

    const Ice::ObjectAdapterIPtr _adapter;
    const bool _dispatcher;
    const Ice::LoggerPtr _logger;
    const TraceLevelsPtr _traceLevels;

    int _requestId;
    std::map<OutgoingAsyncBasePtr, Ice::Int> _sendAsyncRequests;
    std::map<Ice::Int, OutgoingAsyncBasePtr> _asyncRequests;
};
ICE_DEFINE_PTR(CollocatedRequestHandlerPtr, CollocatedRequestHandler);

}

#endif
