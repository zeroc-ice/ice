//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_INVOKER_H
#define ICE_INVOKER_H

#include <IceUtil/Shared.h>
#include <Ice/ConnectionIF.h>
#include <Ice/ReferenceF.h>
#include <Ice/OutgoingAsyncF.h>
#include <Ice/RequestHandlerF.h>
#include <Ice/LocalException.h>
#include <Ice/OperationMode.h>

namespace IceInternal
{

class IInvoker
{
public:

    virtual std::pair<AsyncStatus, RequestHandlerPtr> invoke(const ProxyOutgoingAsyncBasePtr& outgoingAsync) = 0;

    virtual RequestHandlerPtr getRequestHandler() = 0;

    virtual void clearRequestHandler(const RequestHandlerPtr&) = 0;

    virtual int handleException(const ::Ice::Exception&, const ::IceInternal::RequestHandlerPtr&, ::Ice::OperationMode,
                                bool, int&) = 0;
};

class ProxyInvoker : public IInvoker
{
public:

    ProxyInvoker(const ReferencePtr&);

    virtual std::pair<AsyncStatus, RequestHandlerPtr> invoke(const ProxyOutgoingAsyncBasePtr& outgoingAsync);

    virtual RequestHandlerPtr getRequestHandler();

    virtual void clearRequestHandler(const RequestHandlerPtr&);

    virtual int handleException(const ::Ice::Exception&, const ::IceInternal::RequestHandlerPtr&, ::Ice::OperationMode,
                         bool, int&);

private:

    std::mutex _mutex;
    const ReferencePtr _reference;
    RequestHandlerPtr _requestHandler;
};

// TODO: Add CommunicatorInvoker to deal with CommunicatorFlushBatchAsync? FixedProxyInvoker? Non-cached invoker
// if _reference->getCachedConnection() == false? We could potentially get rid of the OutgoingXxxAsync hierarchy
// and only keep a single OutgoingAsync class.

}

#endif
