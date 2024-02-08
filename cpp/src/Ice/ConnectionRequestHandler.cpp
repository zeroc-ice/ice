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

AsyncStatus
ConnectionRequestHandler::sendAsyncRequest(const ProxyOutgoingAsyncBasePtr& out)
{
    return out->invokeRemote(_connection, _compress, _response);
}

void
ConnectionRequestHandler::asyncRequestCanceled(const OutgoingAsyncBasePtr& outAsync, std::exception_ptr ex)
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
