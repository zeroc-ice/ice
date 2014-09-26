// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ConnectionRequestHandler.h>
#include <Ice/Proxy.h>
#include <Ice/Reference.h>
#include <Ice/ConnectionI.h>
#include <Ice/RouterInfo.h>
#include <Ice/Outgoing.h>
#include <Ice/OutgoingAsync.h>

using namespace std;
using namespace IceInternal;

ConnectionRequestHandler::ConnectionRequestHandler(const ReferencePtr& reference, const Ice::ObjectPrx& proxy) :
    RequestHandler(reference)
{
    _connection = _reference->getConnection(_compress);
    RouterInfoPtr ri = reference->getRouterInfo();
    if(ri)
    {
        ri->addProxy(proxy);
    }
}

ConnectionRequestHandler::ConnectionRequestHandler(const ReferencePtr& reference,
                                                   const Ice::ConnectionIPtr& connection,
                                                   bool compress) :
    RequestHandler(reference),
    _connection(connection),
    _compress(compress)
{
}

RequestHandlerPtr
ConnectionRequestHandler::connect()
{
    assert(false); // This request handler is only created after connection binding.
    return 0;
}

RequestHandlerPtr
ConnectionRequestHandler::update(const RequestHandlerPtr& previousHandler, const RequestHandlerPtr& newHandler)
{
    assert(previousHandler);
    try
    {
        if(previousHandler.get() == this)
        {
            return newHandler;
        }
        else if(previousHandler->getConnection() == _connection)
        {
            //
            // If both request handlers point to the same connection, we also
            // update the request handler. See bug ICE-5489 for reasons why
            // this can be useful.
            //
            return newHandler;
        }
    }
    catch(const Ice::Exception&)
    {
        // Ignore.
    }
    return this;
}

void
ConnectionRequestHandler::prepareBatchRequest(BasicStream* out)
{
    _connection->prepareBatchRequest(out);
}

void
ConnectionRequestHandler::finishBatchRequest(BasicStream* out)
{
    _connection->finishBatchRequest(out, _compress);
}

void
ConnectionRequestHandler::abortBatchRequest()
{
    _connection->abortBatchRequest();
}

bool
ConnectionRequestHandler::sendRequest(OutgoingMessageCallback* out)
{
    return out->send(_connection, _compress, _response) && !_response; // Finished if sent and no response
}

AsyncStatus
ConnectionRequestHandler::sendAsyncRequest(const OutgoingAsyncMessageCallbackPtr& out)
{
    return out->__send(_connection, _compress, _response);
}

void
ConnectionRequestHandler::requestTimedOut(OutgoingMessageCallback* out)
{
    _connection->requestTimedOut(out);
}

void
ConnectionRequestHandler::asyncRequestTimedOut(const OutgoingAsyncMessageCallbackPtr& outAsync)
{
    _connection->asyncRequestTimedOut(outAsync);
}

Ice::ConnectionIPtr
ConnectionRequestHandler::getConnection()
{
    return _connection;
}

Ice::ConnectionIPtr
ConnectionRequestHandler::waitForConnection()
{
    return _connection;
}
