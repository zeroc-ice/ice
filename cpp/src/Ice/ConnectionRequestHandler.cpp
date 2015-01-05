// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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

ConnectionRequestHandler::ConnectionRequestHandler(const ReferencePtr& reference,
                                                   const Ice::ConnectionIPtr& connection,
                                                   bool compress) :
    RequestHandler(reference),
    _connection(connection),
    _compress(compress)
{
}

RequestHandlerPtr
ConnectionRequestHandler::connect(const Ice::ObjectPrx&)
{
    return this;
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
ConnectionRequestHandler::sendRequest(OutgoingBase* out)
{
    return out->send(_connection, _compress, _response) && !_response; // Finished if sent and no response
}

AsyncStatus
ConnectionRequestHandler::sendAsyncRequest(const OutgoingAsyncBasePtr& out)
{
    return out->send(_connection, _compress, _response);
}

void
ConnectionRequestHandler::requestCanceled(OutgoingBase* out, const Ice::LocalException& ex)
{
    _connection->requestCanceled(out, ex);
}

void
ConnectionRequestHandler::asyncRequestCanceled(const OutgoingAsyncBasePtr& outAsync, const Ice::LocalException& ex)
{
    _connection->asyncRequestCanceled(outAsync, ex);
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
