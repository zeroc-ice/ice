// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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
    RequestHandler(reference),
    _proxy(proxy.get())
{
// COMPILERFIX: Without the catch/rethrow C++Builder 2007 can get access violations.
#ifdef __BCPLUSPLUS__ 
    try
    {
#endif
        _connection = _reference->getConnection(_compress);
#ifdef __BCPLUSPLUS__
    }
    catch(const Ice::LocalException&)
    {
        throw;
    }
#endif
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

Ice::ConnectionI*
ConnectionRequestHandler::sendRequest(Outgoing* out)
{
    if(!_connection->sendRequest(out, _compress, _response) || _response)
    {
        return _connection.get(); // The request has been sent or we're expecting a response.
    }
    else
    {
        return 0; // The request hasn't been sent yet.
    }
}

AsyncStatus
ConnectionRequestHandler::sendAsyncRequest(const OutgoingAsyncPtr& out)
{
    return _connection->sendAsyncRequest(out, _compress, _response);
}

bool
ConnectionRequestHandler::flushBatchRequests(BatchOutgoing* out)
{
    return _connection->flushBatchRequests(out);
}

AsyncStatus
ConnectionRequestHandler::flushAsyncBatchRequests(const BatchOutgoingAsyncPtr& out)
{
    return _connection->flushAsyncBatchRequests(out);
}

Ice::ConnectionIPtr
ConnectionRequestHandler::getConnection(bool wait)
{
    return _connection;
}

IceProxy::Ice::Object*
ConnectionRequestHandler::getProxy() const
{
    return _proxy;
}
