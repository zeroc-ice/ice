// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

class Outgoing;
class BatchOutgoing;
class OutgoingAsync;
class BatchOutgoingAsync;

class CollocatedRequestHandler : public RequestHandler, public ResponseHandler, private IceUtil::Monitor<IceUtil::Mutex>
{
public:

    CollocatedRequestHandler(const ReferencePtr&, const Ice::ObjectAdapterPtr&);

    virtual ~CollocatedRequestHandler();

    virtual void prepareBatchRequest(BasicStream*);
    virtual void finishBatchRequest(BasicStream*);
    virtual void abortBatchRequest();

    virtual bool sendRequest(OutgoingMessageCallback*);
    virtual AsyncStatus sendAsyncRequest(const OutgoingAsyncMessageCallbackPtr&);

    virtual void requestTimedOut(OutgoingMessageCallback*);
    virtual void asyncRequestTimedOut(const OutgoingAsyncMessageCallbackPtr&);

    virtual void sendResponse(Ice::Int, BasicStream*, Ice::Byte);
    virtual void sendNoResponse();
    virtual void invokeException(Ice::Int, const Ice::LocalException&, int);

    const ReferencePtr& getReference() const { return _reference; } // Inlined for performances.

    virtual Ice::ConnectionIPtr getConnection(bool);
    
    void invokeRequest(Outgoing*);
    AsyncStatus invokeAsyncRequest(OutgoingAsync*);
    void invokeBatchRequests(BatchOutgoing*);
    AsyncStatus invokeAsyncBatchRequests(BatchOutgoingAsync*);

    bool sent(OutgoingMessageCallback*);
    bool sentAsync(OutgoingAsyncMessageCallback*);

    void invokeAll(BasicStream*, Ice::Int, Ice::Int, bool);

private:

    void handleException(Ice::Int, const Ice::Exception&);
    
    const Ice::ObjectAdapterIPtr _adapter;
    const bool _dispatcher;
    const Ice::LoggerPtr _logger;
    const TraceLevelsPtr _traceLevels;
    const bool _batchAutoFlush;

    int _requestId;

    std::map<OutgoingMessageCallback*, Ice::Int> _sendRequests;
    std::map<OutgoingAsyncMessageCallbackPtr, Ice::Int> _sendAsyncRequests;

    std::map<Ice::Int, Outgoing*> _requests;
    std::map<Ice::Int, OutgoingAsyncPtr> _asyncRequests;

    bool _batchStreamInUse;
    int _batchRequestNum;
    BasicStream _batchStream;
    size_t _batchMarker;
};
typedef IceUtil::Handle<CollocatedRequestHandler> CollocatedRequestHandlerPtr;

}

#endif
