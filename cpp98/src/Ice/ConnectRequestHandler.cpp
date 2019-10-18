//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/ConnectRequestHandler.h>
#include <Ice/ConnectionRequestHandler.h>
#include <Ice/RequestHandlerFactory.h>
#include <Ice/Instance.h>
#include <Ice/Proxy.h>
#include <Ice/ConnectionI.h>
#include <Ice/RouterInfo.h>
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
    return _requestHandler ? _requestHandler : ICE_SHARED_FROM_THIS;
}

RequestHandlerPtr
ConnectRequestHandler::update(const RequestHandlerPtr& previousHandler, const RequestHandlerPtr& newHandler)
{
    return previousHandler.get() == this ? newHandler : ICE_SHARED_FROM_THIS;
}

AsyncStatus
ConnectRequestHandler::sendAsyncRequest(const ProxyOutgoingAsyncBasePtr& out)
{
    {
        Lock sync(*this);
        if(!_initialized)
        {
            out->cancelable(ICE_SHARED_FROM_THIS); // This will throw if the request is canceled
        }

        if(!initialized())
        {
            _requests.push_back(out);
            return AsyncStatusQueued;
        }
    }
    return out->invokeRemote(_connection, _compress, _response);
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
            for(deque<ProxyOutgoingAsyncBasePtr>::iterator p = _requests.begin(); p != _requests.end(); ++p)
            {
                if(p->get() == outAsync.get())
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
    //
    // First check for the connection, it's important otherwise the user could first get a connection
    // and then the exception if he tries to obtain the proxy cached connection mutiple times (the
    // exception can be set after the connection is set if the flush of pending requests fails).
    //
    if(_connection)
    {
        return _connection;
    }
    else if(_exception)
    {
        _exception->ice_throw();
    }
    return ICE_NULLPTR;
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
        assert(!_flushing && !_exception && !_connection);
        _connection = connection;
        _compress = compress;
    }

    //
    // If this proxy is for a non-local object, and we are using a router, then
    // add this proxy to the router info object.
    //
    RouterInfoPtr ri = _reference->getRouterInfo();
    if(ri && !ri->addProxy(_proxy, ICE_SHARED_FROM_THIS))
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
    {
        Lock sync(*this);
        assert(!_flushing && !_initialized && !_exception);
        _flushing = true; // Ensures request handler is removed before processing new requests.
        ICE_SET_EXCEPTION_FROM_CLONE(_exception, ex.ice_clone());
    }

    //
    // NOTE: remove the request handler *before* notifying the requests that the connection
    // failed. It's important to ensure that future invocations will obtain a new connect
    // request handler once invocations are notified.
    //
    try
    {
        _reference->getInstance()->requestHandlerFactory()->removeRequestHandler(_reference, ICE_SHARED_FROM_THIS);
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
        // Ignore
    }

    for(deque<ProxyOutgoingAsyncBasePtr>::const_iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
        if((*p)->exception(ex))
        {
            (*p)->invokeExceptionAsync();
        }
    }
    _requests.clear();

    {
        Lock sync(*this);
        _flushing = false;
        _proxies.clear();
        _proxy = 0; // Break cyclic reference count.
        notifyAll();
    }
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
        while(_flushing)
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
    IceInternal::UniquePtr<Ice::LocalException> exception;
#endif
    while(!_requests.empty()) // _requests is immutable when _flushing = true
    {
        ProxyOutgoingAsyncBasePtr& req = _requests.front();
        try
        {
            if(req->invokeRemote(_connection, _compress, _response) & AsyncStatusInvokeSentCallback)
            {
                req->invokeSentAsync();
            }
        }
        catch(const RetryException& ex)
        {
            ICE_SET_EXCEPTION_FROM_CLONE(exception, ex.get()->ice_clone());

            // Remove the request handler before retrying.
            _reference->getInstance()->requestHandlerFactory()->removeRequestHandler(_reference, ICE_SHARED_FROM_THIS);

            req->retryException(*exception);
        }
        catch(const Ice::LocalException& ex)
        {
            ICE_SET_EXCEPTION_FROM_CLONE(exception, ex.ice_clone());

            if(req->exception(ex))
            {
                req->invokeExceptionAsync();
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
            (*p)->_updateRequestHandler(ICE_SHARED_FROM_THIS, _requestHandler);
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
        _reference->getInstance()->requestHandlerFactory()->removeRequestHandler(_reference, ICE_SHARED_FROM_THIS);

        _proxies.clear();
        _proxy = ICE_NULLPTR; // Break cyclic reference count.
        notifyAll();
    }
}
