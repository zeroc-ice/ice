//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/ConnectionRequestHandler.h>
#include <Ice/Proxy.h>
#include <Ice/Reference.h>
#include <Ice/ConnectionI.h>
#include <Ice/RouterInfo.h>
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
    return ICE_SHARED_FROM_THIS;
}

AsyncStatus
ConnectionRequestHandler::sendAsyncRequest(const ProxyOutgoingAsyncBasePtr& out)
{
    return out->invokeRemote(_connection, _compress, _response);
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
