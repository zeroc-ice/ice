// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_REQUEST_HANDLER_H
#define ICE_REQUEST_HANDLER_H

#include <IceUtil/Shared.h>
#include <IceUtil/UniquePtr.h>

#include <Ice/RequestHandlerF.h>
#include <Ice/ReferenceF.h>
#include <Ice/OutgoingAsyncF.h>
#include <Ice/ProxyF.h>
#include <Ice/ConnectionIF.h>

namespace Ice
{

class LocalException;

};

namespace IceInternal
{

class BasicStream;

class OutgoingBase;

//
// An exception wrapper, which is used to notify that the request
// handler should be cleared and the invocation retried.
//
class RetryException
{
public:

    RetryException(const Ice::LocalException&);
    RetryException(const RetryException&);

    const Ice::LocalException* get() const;

private:

    IceUtil::UniquePtr<Ice::LocalException> _ex;
};

class CancellationHandler : virtual public IceUtil::Shared
{
public:

    virtual void requestCanceled(OutgoingBase*, const Ice::LocalException&) = 0;
    virtual void asyncRequestCanceled(const OutgoingAsyncBasePtr&, const Ice::LocalException&) = 0;
};

class RequestHandler : public CancellationHandler
{
public:

    virtual RequestHandlerPtr connect(const Ice::ObjectPrx&) = 0;
    virtual RequestHandlerPtr update(const RequestHandlerPtr&, const RequestHandlerPtr&) = 0;

    virtual void prepareBatchRequest(BasicStream*) = 0;
    virtual void finishBatchRequest(BasicStream*) = 0;
    virtual void abortBatchRequest() = 0;

    virtual bool sendRequest(OutgoingBase*) = 0;
    virtual AsyncStatus sendAsyncRequest(const OutgoingAsyncBasePtr&) = 0;

    const ReferencePtr& getReference() const { return _reference; } // Inlined for performances.

    virtual Ice::ConnectionIPtr getConnection() = 0;
    virtual Ice::ConnectionIPtr waitForConnection() = 0;

protected:

    RequestHandler(const ReferencePtr&);
    const ReferencePtr _reference;
    const bool _response;
};

}

#endif
