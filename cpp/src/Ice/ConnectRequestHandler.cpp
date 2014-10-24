// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ConnectRequestHandler.h>
#include <Ice/ConnectionRequestHandler.h>
#include <Ice/RequestHandlerFactory.h>
#include <Ice/Instance.h>
#include <Ice/Proxy.h>
#include <Ice/ConnectionI.h>
#include <Ice/RouterInfo.h>
#include <Ice/Outgoing.h>
#include <Ice/OutgoingAsync.h>
#include <Ice/Protocol.h>
#include <Ice/Properties.h>
#include <Ice/ThreadPool.h>

using namespace std;
using namespace IceInternal;

ConnectRequestHandler::ConnectRequestHandler(const ReferencePtr& ref, const Ice::ObjectPrx& proxy) :
    RequestHandler(ref),
    _connect(true),
    _proxy(proxy),
    _batchAutoFlush(
        ref->getInstance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.BatchAutoFlush", 1) > 0),
    _initialized(false),
    _flushing(false),
    _batchRequestInProgress(false),
    _batchRequestsSize(sizeof(requestBatchHdr)),
    _batchStream(ref->getInstance().get(), Ice::currentProtocolEncoding, _batchAutoFlush)
{
}

ConnectRequestHandler::~ConnectRequestHandler()
{
}

RequestHandlerPtr
ConnectRequestHandler::connect(const Ice::ObjectPrx& proxy)
{
    //
    // Initiate the connection if connect() is called by the proxy that
    // created the handler.
    //
    if(proxy.get() == _proxy.get() && _connect) 
    {
        _connect = false; // Call getConnection only once
        _reference->getConnection(this);
    }

    try
    {
        Lock sync(*this);
        if(!initialized())
        {
            _proxies.insert(proxy);
            return this;
        }
    }
    catch(const Ice::LocalException&)
    {
        proxy->__setRequestHandler(this, 0);
        throw;
    }

    if(_connectionRequestHandler)
    {
        proxy->__setRequestHandler(this, _connectionRequestHandler);
        return _connectionRequestHandler;
    }
    else
    {
        return this;
    }
}

RequestHandlerPtr
ConnectRequestHandler::update(const RequestHandlerPtr& previousHandler, const RequestHandlerPtr& newHandler)
{
    return previousHandler.get() == this ? newHandler : this;
}

void
ConnectRequestHandler::prepareBatchRequest(BasicStream* os)
{
    {
        Lock sync(*this);
        while(_batchRequestInProgress)
        {
            wait();
        }

        try
        {
            if(!initialized())
            {
                _batchRequestInProgress = true;
                _batchStream.swap(*os);
                return;
            }
        }
        catch(const Ice::LocalException& ex)
        {
            throw RetryException(ex);
        }
    }
    _connection->prepareBatchRequest(os);
}

void
ConnectRequestHandler::finishBatchRequest(BasicStream* os)
{
    {
        Lock sync(*this);
        if(!initialized()) // This can't throw until _batchRequestInProgress = false
        {
            assert(_batchRequestInProgress);
            _batchRequestInProgress = false;
            notifyAll();

            _batchStream.swap(*os);

            if(!_batchAutoFlush &&
               _batchStream.b.size() + _batchRequestsSize > _reference->getInstance()->messageSizeMax())
            {
                Ex::throwMemoryLimitException(__FILE__, __LINE__, _batchStream.b.size() + _batchRequestsSize,
                                              _reference->getInstance()->messageSizeMax());
            }

            _batchRequestsSize += _batchStream.b.size();

            Request req;
            req.os = new BasicStream(_reference->getInstance().get(), Ice::currentProtocolEncoding, _batchAutoFlush);
            req.os->swap(_batchStream);
            _requests.push_back(req);
            return;
        }
    }
    _connection->finishBatchRequest(os, _compress);
}

void
ConnectRequestHandler::abortBatchRequest()
{
    {
        Lock sync(*this);
        if(!initialized()) // This can't throw until _batchRequestInProgress = false
        {
            assert(_batchRequestInProgress);
            _batchRequestInProgress = false;
            notifyAll();

            BasicStream dummy(_reference->getInstance().get(), Ice::currentProtocolEncoding, _batchAutoFlush);
            _batchStream.swap(dummy);
            _batchRequestsSize = sizeof(requestBatchHdr);
            return;
        }
    }
    _connection->abortBatchRequest();
}

bool
ConnectRequestHandler::sendRequest(OutgoingBase* out)
{
    {
        Lock sync(*this);
        try
        {
            if(!initialized())
            {
                Request req;
                req.out = out;
                _requests.push_back(req);
                return false; // Not sent
            }
        }
        catch(const Ice::LocalException& ex)
        {
            throw RetryException(ex);
        }
    }
    return out->send(_connection, _compress, _response) && !_response; // Finished if sent and no response.
}

AsyncStatus
ConnectRequestHandler::sendAsyncRequest(const OutgoingAsyncBasePtr& out)
{
    {
        Lock sync(*this);
        if(!_initialized)
        {
            out->cancelable(this); // This will throw if the request is canceled
        }

        try
        {
            if(!initialized())
            {
                Request req;
                req.outAsync = out;
                _requests.push_back(req);
                return AsyncStatusQueued;
            }
        }
        catch(const Ice::LocalException& ex)
        {
            throw RetryException(ex);
        }
    }
    return out->send(_connection, _compress, _response);
}

void
ConnectRequestHandler::requestCanceled(OutgoingBase* out, const Ice::LocalException& ex)
{
    {
        Lock sync(*this);
        if(_exception.get())
        {
            return; // The request has been notified of a failure already.
        }

        if(!initialized())
        {
            for(deque<Request>::iterator p = _requests.begin(); p != _requests.end(); ++p)
            {
                if(p->out == out)
                {
                    out->completed(ex);
                    _requests.erase(p);
                    return;
                }
            }
            assert(false); // The request has to be queued if it timed out and we're not initialized yet.
        }
    }
    _connection->requestCanceled(out, ex);
}

void
ConnectRequestHandler::asyncRequestCanceled(const OutgoingAsyncBasePtr& outAsync, const Ice::LocalException& ex)
{
    {
        Lock sync(*this);
        if(_exception.get())
        {
            return; // The request has been notified of a failure already.
        }

        if(!initialized())
        {
            for(deque<Request>::iterator p = _requests.begin(); p != _requests.end(); ++p)
            {
                if(p->outAsync.get() == outAsync.get())
                {
                    _requests.erase(p);
                    if(outAsync->completed(ex))
                    {
                        outAsync->invokeCompletedAsync();
                    }
                    return;
                }
            }
        }
    }
    _connection->asyncRequestCanceled(outAsync, ex);
}

Ice::ConnectionIPtr
ConnectRequestHandler::getConnection()
{
    Lock sync(*this);
    if(_exception.get())
    {
        _exception->ice_throw();
        return 0; // Keep the compiler happy.
    }
    else
    {
        return _connection;
    }
}

Ice::ConnectionIPtr
ConnectRequestHandler::waitForConnection()
{
    Lock sync(*this);
    if(_exception.get())
    {
        throw RetryException(*_exception.get());
    }

    //
    // Wait for the connection establishment to complete or fail.
    //
    while(!_initialized && !_exception.get())
    {
        wait();
    }

    if(_exception.get())
    {
        _exception->ice_throw();
        return 0; // Keep the compiler happy.
    }
    else
    {
        return _connection;
    }
}

void
ConnectRequestHandler::setConnection(const Ice::ConnectionIPtr& connection, bool compress)
{
    {
        Lock sync(*this);
        assert(!_exception.get() && !_connection);
        _connection = connection;
        _compress = compress;
    }

    //
    // If this proxy is for a non-local object, and we are using a router, then
    // add this proxy to the router info object.
    //
    RouterInfoPtr ri = _reference->getRouterInfo();
    if(ri && !ri->addProxy(_proxy, this))
    {
        return; // The request handler will be initialized once addProxy returns.
    }

    //
    // We can now send the queued requests.
    //
    flushRequests();
}

void
ConnectRequestHandler::setException(const Ice::LocalException& ex)
{
    Lock sync(*this);
    assert(!_initialized && !_exception.get());
    _exception.reset(ex.ice_clone());
    _proxies.clear();
    _proxy = 0; // Break cyclic reference count.

    flushRequestsWithException();

    try
    {
        _reference->getInstance()->requestHandlerFactory()->removeRequestHandler(_reference, this);
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
        // Ignore
    }
    notifyAll();
}

void
ConnectRequestHandler::addedProxy()
{
    //
    // The proxy was added to the router info, we're now ready to send the
    // queued requests.
    //
    flushRequests();
}

bool
ConnectRequestHandler::initialized()
{
    // Must be called with the mutex locked.

    if(_initialized)
    {
        assert(_connection);
        return true;
    }
    else
    {
        while(_flushing && !_exception.get())
        {
            wait();
        }

        if(_exception.get())
        {
            _exception->ice_throw();
            return false; // Keep the compiler happy.
        }
        else
        {
            return _initialized;
        }
    }
}

void
ConnectRequestHandler::flushRequests()
{
    {
        Lock sync(*this);
        assert(_connection && !_initialized);

        while(_batchRequestInProgress)
        {
            wait();
        }

        //
        // We set the _flushing flag to true to prevent any additional queuing. Callers
        // might block for a little while as the queued requests are being sent but this
        // shouldn't be an issue as the request sends are non-blocking.
        //
        _flushing = true;
    }

    try
    {
        while(!_requests.empty()) // _requests is immutable when _flushing = true
        {
            Request& req = _requests.front();
            if(req.out)
            {
                req.out->send(_connection, _compress, _response);
            }
            else if(req.outAsync)
            {
                if(req.outAsync->send(_connection, _compress, _response) & AsyncStatusInvokeSentCallback)
                {
                    req.outAsync->invokeSentAsync();
                }
            }
            else
            {
                BasicStream os(req.os->instance(), Ice::currentProtocolEncoding);
                _connection->prepareBatchRequest(&os);
                try
                {
                    const Ice::Byte* bytes;
                    req.os->i = req.os->b.begin();
                    req.os->readBlob(bytes, req.os->b.size());
                    os.writeBlob(bytes, req.os->b.size());
                }
                catch(const Ice::LocalException&)
                {
                    _connection->abortBatchRequest();
                    throw;
                }
                _connection->finishBatchRequest(&os, _compress);
                delete req.os;
            }
            _requests.pop_front();
        }
    }
    catch(const RetryException& ex)
    {
        //
        // If the connection dies shortly after connection
        // establishment, we don't systematically retry on
        // RetryException. We handle the exception like it
        // was an exception that occured while sending the
        // request.
        //
        Lock sync(*this);
        assert(!_exception.get() && !_requests.empty());
        _exception.reset(ex.get()->ice_clone());
        flushRequestsWithException();
    }
    catch(const Ice::LocalException& ex)
    {
        Lock sync(*this);
        assert(!_exception.get() && !_requests.empty());
        _exception.reset(ex.ice_clone());
        flushRequestsWithException();
    }

    //
    // If we aren't caching the connection, don't bother creating a
    // connection request handler. Otherwise, update the proxies
    // request handler to use the more efficient connection request
    // handler.
    //
    if(_reference->getCacheConnection() && !_exception.get())
    {
        _connectionRequestHandler = new ConnectionRequestHandler(_reference, _connection, _compress);
        for(set<Ice::ObjectPrx>::const_iterator p = _proxies.begin(); p != _proxies.end(); ++p)
        {
            (*p)->__setRequestHandler(this, _connectionRequestHandler);
        }
    }

    {
        Lock sync(*this);
        assert(!_initialized);
        if(!_exception.get())
        {
            _initialized = true;
            _flushing = false;
        }
        try
        {
            _reference->getInstance()->requestHandlerFactory()->removeRequestHandler(_reference, this);
        }
        catch(const Ice::CommunicatorDestroyedException&)
        {
            // Ignore
        }
        _proxies.clear();
        _proxy = 0; // Break cyclic reference count.
        notifyAll();
    }
}

void
ConnectRequestHandler::flushRequestsWithException()
{
    assert(_exception.get());

    for(deque<Request>::const_iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
        if(p->out)
        {
            p->out->completed(*_exception.get());
        }
        else if(p->outAsync)
        {
            if(p->outAsync->completed(*_exception.get()))
            {
                p->outAsync->invokeCompletedAsync();
            }
        }
        else
        {
            assert(p->os);
            delete p->os;
        }
    }
    _requests.clear();
}
