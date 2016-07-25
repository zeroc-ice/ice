// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

#ifndef ICE_CPP11_MAPPING
IceUtil::Shared* IceInternal::upCast(ConnectRequestHandler* p) { return p; }
#endif

ConnectRequestHandler::ConnectRequestHandler(const ReferencePtr& ref, const Ice::ObjectPrxPtr& proxy) :
    RequestHandler(ref),
    _proxy(proxy),
    _initialized(false),
    _flushing(false)
{
}

RequestHandlerPtr
ConnectRequestHandler::connect(const Ice::ObjectPrxPtr& proxy)
{
    Lock sync(*this);
    if(!initialized())
    {
        _proxies.insert(proxy);
    }
    return _requestHandler ? _requestHandler : shared_from_this();
}

RequestHandlerPtr
ConnectRequestHandler::update(const RequestHandlerPtr& previousHandler, const RequestHandlerPtr& newHandler)
{
    return previousHandler.get() == this ? newHandler : shared_from_this();
}

bool
ConnectRequestHandler::sendRequest(ProxyOutgoingBase* out)
{
    {
        Lock sync(*this);
        if(!initialized())
        {
            Request req;
            req.out = out;
            _requests.push_back(req);
            return false; // Not sent
        }
    }
    return out->invokeRemote(_connection, _compress, _response) && !_response; // Finished if sent and no response.
}

AsyncStatus
ConnectRequestHandler::sendAsyncRequest(const ProxyOutgoingAsyncBasePtr& out)
{
    {
        Lock sync(*this);
        if(!_initialized)
        {
            out->cancelable(shared_from_this()); // This will throw if the request is canceled
        }

        if(!initialized())
        {
            Request req;
            req.outAsync = out;
            _requests.push_back(req);
            return AsyncStatusQueued;
        }
    }
    return out->invokeRemote(_connection, _compress, _response);
}

void
ConnectRequestHandler::requestCanceled(OutgoingBase* out, const Ice::LocalException& ex)
{
    {
        Lock sync(*this);
        if(_exception)
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
        if(_exception)
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
                    if(outAsync->exception(ex))
                    {
                        outAsync->invokeExceptionAsync();
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
    if(_exception)
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
    if(_exception)
    {
        throw RetryException(*_exception);
    }
    //
    // Wait for the connection establishment to complete or fail.
    //
    while(!_initialized && !_exception)
    {
        wait();
    }

    if(_exception)
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
        assert(!_exception && !_connection);
        _connection = connection;
        _compress = compress;
    }

    //
    // If this proxy is for a non-local object, and we are using a router, then
    // add this proxy to the router info object.
    //
    RouterInfoPtr ri = _reference->getRouterInfo();
    if(ri && !ri->addProxy(_proxy, AddProxyCallback::shared_from_this()))
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
    assert(!_initialized && !_exception);
    ICE_SET_EXCEPTION_FROM_CLONE(_exception, ex.ice_clone());

    _proxies.clear();
    _proxy = 0; // Break cyclic reference count.

    //
    // NOTE: remove the request handler *before* notifying the
    // requests that the connection failed. It's important to ensure
    // that future invocations will obtain a new connect request
    // handler once invocations are notified.
    //
    try
    {
        _reference->getInstance()->requestHandlerFactory()->removeRequestHandler(_reference, shared_from_this());
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
        // Ignore
    }


    for(deque<Request>::const_iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
        if(p->out)
        {
            p->out->completed(ex);
        }
        else
        {
            if(p->outAsync->exception(ex))
            {
                p->outAsync->invokeExceptionAsync();
            }
        }
    }

    _requests.clear();
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
        while(_flushing && !_exception)
        {
            wait();
        }

        if(_exception)
        {
            if(_connection)
            {
                //
                // Only throw if the connection didn't get established. If
                // it died after being established, we allow the caller to
                // retry the connection establishment by not throwing here
                // (the connection will throw RetryException).
                //
                return true;
            }
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

        //
        // We set the _flushing flag to true to prevent any additional queuing. Callers
        // might block for a little while as the queued requests are being sent but this
        // shouldn't be an issue as the request sends are non-blocking.
        //
        _flushing = true;
    }

#ifdef ICE_CPP11_MAPPING
    std::unique_ptr<Ice::LocalException> exception;
#else
    IceUtil::UniquePtr<Ice::LocalException> exception;
#endif
    while(!_requests.empty()) // _requests is immutable when _flushing = true
    {
        Request& req = _requests.front();
        try
        {
            if(req.out)
            {
                req.out->invokeRemote(_connection, _compress, _response);
            }
            else if(req.outAsync->invokeRemote(_connection, _compress, _response) & AsyncStatusInvokeSentCallback)
            {
                req.outAsync->invokeSentAsync();
            }
        }
        catch(const RetryException& ex)
        {
            ICE_SET_EXCEPTION_FROM_CLONE(exception, ex.get()->ice_clone());

            // Remove the request handler before retrying.
            _reference->getInstance()->requestHandlerFactory()->removeRequestHandler(_reference, shared_from_this());

            if(req.out)
            {
                req.out->retryException(*exception);
            }
            else
            {
                req.outAsync->retryException(*exception);
            }
        }
        catch(const Ice::LocalException& ex)
        {
            ICE_SET_EXCEPTION_FROM_CLONE(exception, ex.ice_clone());
            if(req.out)
            {
                req.out->completed(ex);
            }
            else if(req.outAsync->exception(ex))
            {
                req.outAsync->invokeExceptionAsync();
            }
        }
        _requests.pop_front();
    }

    //
    // If we aren't caching the connection, don't bother creating a
    // connection request handler. Otherwise, update the proxies
    // request handler to use the more efficient connection request
    // handler.
    //
    if(_reference->getCacheConnection() && !exception)
    {
        _requestHandler = ICE_MAKE_SHARED(ConnectionRequestHandler, _reference, _connection, _compress);
        for(set<Ice::ObjectPrxPtr>::const_iterator p = _proxies.begin(); p != _proxies.end(); ++p)
        {
            (*p)->__updateRequestHandler(shared_from_this(), _requestHandler);
        }
    }

    {
        Lock sync(*this);
        assert(!_initialized);
#ifdef ICE_CPP11_MAPPING
        swap(_exception, exception);
#else
        _exception.swap(exception);
#endif
        _initialized = !_exception;
        _flushing = false;

        //
        // Only remove once all the requests are flushed to
        // guarantee serialization.
        //
        _reference->getInstance()->requestHandlerFactory()->removeRequestHandler(_reference, shared_from_this());

        _proxies.clear();
        _proxy = ICE_NULLPTR; // Break cyclic reference count.
        notifyAll();
    }
}
