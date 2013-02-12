// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_REQUEST_HANDLER_H
#define ICE_REQUEST_HANDLER_H

#include <IceUtil/Shared.h>

#include <Ice/RequestHandlerF.h>
#include <Ice/ReferenceF.h>
#include <Ice/OutgoingAsyncF.h>
#include <Ice/ProxyF.h>
#include <Ice/ConnectionIF.h>

namespace IceInternal
{

class BasicStream;
class Outgoing;
class BatchOutgoing;

class RequestHandler : virtual public ::IceUtil::Shared
{
public:

    virtual ~RequestHandler();

    virtual void prepareBatchRequest(BasicStream*) = 0;
    virtual void finishBatchRequest(BasicStream*) = 0;
    virtual void abortBatchRequest() = 0;

    virtual Ice::ConnectionI* sendRequest(Outgoing*) = 0;
    virtual AsyncStatus sendAsyncRequest(const OutgoingAsyncPtr&) = 0;

    virtual bool flushBatchRequests(BatchOutgoing*) = 0;
    virtual AsyncStatus flushAsyncBatchRequests(const BatchOutgoingAsyncPtr&) = 0;

    const ReferencePtr& getReference() const { return _reference; } // Inlined for performances.

    virtual Ice::ConnectionIPtr getConnection(bool) = 0;
    
protected:

    RequestHandler(const ReferencePtr&);
    const ReferencePtr _reference;
    const bool _response;
};

}

#endif
