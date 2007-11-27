// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    _response(reference->getMode() == Reference::ModeTwoway)
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
    _response(reference->getMode() == Reference::ModeTwoway),
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
    return (!_connection->sendRequest(out, _compress, _response) || _response) ? _connection.get() : 0;
}

void
ConnectionRequestHandler::sendAsyncRequest(const OutgoingAsyncPtr& out)
{
    try
    {
        _connection->sendAsyncRequest(out, _compress, _response);
    }
    catch(const LocalExceptionWrapper& ex)
    {
        out->__finished(ex);
    }
    catch(const Ice::LocalException& ex)
    {
        out->__finished(ex);
    }
}

bool
ConnectionRequestHandler::flushBatchRequests(BatchOutgoing* out)
{
    return _connection->flushBatchRequests(out);
}

void
ConnectionRequestHandler::flushAsyncBatchRequests(const BatchOutgoingAsyncPtr& out)
{
    try
    {
        _connection->flushAsyncBatchRequests(out);
    }
    catch(const Ice::LocalException& ex)
    {
        out->__finished(ex);
    }
}

Ice::ConnectionIPtr
ConnectionRequestHandler::getConnection(bool wait)
{
    return _connection;
}
