// Copyright (c) ZeroC, Inc.

#include "ConnectRequestHandler.h"
#include "ConnectionI.h"
#include "Ice/OutgoingAsync.h"
#include "RouterInfo.h"

using namespace std;
using namespace IceInternal;

ConnectRequestHandler::ConnectRequestHandler(const ReferencePtr& ref) : RequestHandler(ref) {}

AsyncStatus
ConnectRequestHandler::sendAsyncRequest(const ProxyOutgoingAsyncBasePtr& out)
{
    {
        unique_lock lock(_mutex);
        if (!_initialized)
        {
            out->cancelable(shared_from_this()); // This will throw if the request is canceled
        }

        if (!initialized(lock))
        {
            _requests.push_back(out);
            return AsyncStatusQueued;
        }
    }
    return out->invokeRemote(_connection, _compress, _response);
}

void
ConnectRequestHandler::asyncRequestCanceled(const OutgoingAsyncBasePtr& outAsync, exception_ptr ex)
{
    {
        unique_lock lock(_mutex);
        if (_exception)
        {
            return; // The request has been notified of a failure already.
        }

        if (!initialized(lock))
        {
            for (auto p = _requests.begin(); p != _requests.end(); ++p)
            {
                if (p->get() == outAsync.get())
                {
                    _requests.erase(p);
                    if (outAsync->exception(ex))
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
    lock_guard lock(_mutex);
    //
    // First check for the connection, it's important otherwise the user could first get a connection
    // and then the exception if he tries to obtain the proxy cached connection mutiple times (the
    // exception can be set after the connection is set if the flush of pending requests fails).
    //
    if (_connection)
    {
        return _connection;
    }
    else if (_exception)
    {
        rethrow_exception(_exception);
    }
    return nullptr;
}

void
ConnectRequestHandler::setConnection(Ice::ConnectionIPtr connection, bool compress)
{
    {
        lock_guard lock(_mutex);
        assert(!_flushing && !_exception && !_connection);
        _connection = std::move(connection);
        _compress = compress;
    }

    //
    // If we are using a router, add this proxy to the router info object.
    //
    RouterInfoPtr ri = _reference->getRouterInfo();

    if (ri)
    {
        auto self = shared_from_this();
        if (!ri->addProxyAsync(
                _reference,
                [self] { self->flushRequests(); },
                [self](exception_ptr ex) { self->setException(ex); }))
        {
            return; // The request handler will be initialized once addProxyAsync completes.
        }
    }

    //
    // We can now send the queued requests.
    //
    flushRequests();
}

void
ConnectRequestHandler::setException(exception_ptr ex)
{
    {
        lock_guard lock(_mutex);
        assert(!_flushing && !_initialized && !_exception);
        _flushing = true; // Ensures request handler is removed before processing new requests.
        _exception = ex;
    }

    for (const auto& request : _requests)
    {
        if (request->exception(ex))
        {
            request->invokeExceptionAsync();
        }
    }

    _requests.clear();

    {
        lock_guard lock(_mutex);
        _flushing = false;
        _conditionVariable.notify_all();
    }
}

bool
ConnectRequestHandler::initialized(unique_lock<mutex>& lock)
{
    // Must be called with the mutex locked.

    if (_initialized)
    {
        assert(_connection);
        return true;
    }
    else
    {
        _conditionVariable.wait(lock, [this] { return !_flushing; });

        if (_exception)
        {
            if (_connection)
            {
                //
                // Only throw if the connection didn't get established. If
                // it died after being established, we allow the caller to
                // retry the connection establishment by not throwing here
                // (the connection will throw RetryException).
                //
                return true;
            }
            rethrow_exception(_exception);
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
        lock_guard lock(_mutex);
        assert(_connection && !_initialized);

        //
        // We set the _flushing flag to true to prevent any additional queuing. Callers
        // might block for a little while as the queued requests are being sent but this
        // shouldn't be an issue as the request sends are non-blocking.
        //
        _flushing = true;
    }

    exception_ptr exception;
    while (!_requests.empty()) // _requests is immutable when _flushing = true
    {
        ProxyOutgoingAsyncBasePtr& req = _requests.front();
        try
        {
            if (req->invokeRemote(_connection, _compress, _response) & AsyncStatusInvokeSentCallback)
            {
                req->invokeSentAsync();
            }
        }
        catch (const RetryException& ex)
        {
            exception = ex.get();
            req->retryException();
        }
        catch (const Ice::LocalException&)
        {
            exception = current_exception();

            if (req->exception(exception))
            {
                req->invokeExceptionAsync();
            }
        }
        _requests.pop_front();
    }

    {
        lock_guard lock(_mutex);
        assert(!_initialized);
        swap(_exception, exception);
        _initialized = !_exception;
        _flushing = false;
        _conditionVariable.notify_all();
    }
}
