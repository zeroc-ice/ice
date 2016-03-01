// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

bool
ConnectionRequestHandler::sendRequest(ProxyOutgoingBase* out)
{
    return out->invokeRemote(_connection, _compress, _response) && !_response; // Finished if sent and no response
}

AsyncStatus
ConnectionRequestHandler::sendAsyncRequest(const ProxyOutgoingAsyncBasePtr& out)
{
    return out->invokeRemote(_connection, _compress, _response);
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
