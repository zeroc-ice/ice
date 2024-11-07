// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.Internal;

internal class ConnectRequestHandler : RequestHandler, Reference.GetConnectionCallback, RouterInfo.AddProxyCallback
{
    internal ConnectRequestHandler(Reference reference)
    {
        _reference = reference;
        _response = reference.isTwoway;
    }

    public int sendAsyncRequest(ProxyOutgoingAsyncBase outAsync)
    {
        lock (_mutex)
        {
            if (!_initialized)
            {
                outAsync.cancelable(this); // This will throw if the request is canceled
            }

            if (!initialized())
            {
                _requests.AddLast(outAsync);
                return OutgoingAsyncBase.AsyncStatusQueued;
            }
        }
        return outAsync.invokeRemote(_connection, _compress, _response);
    }

    public void asyncRequestCanceled(OutgoingAsyncBase outAsync, Ice.LocalException ex)
    {
        lock (_mutex)
        {
            if (_exception != null)
            {
                return; // The request has been notified of a failure already.
            }

            if (!initialized())
            {
                LinkedListNode<ProxyOutgoingAsyncBase> p = _requests.First;
                while (p != null)
                {
                    if (p.Value == outAsync)
                    {
                        _requests.Remove(p);
                        if (outAsync.exception(ex))
                        {
                            outAsync.invokeExceptionAsync();
                        }
                        return;
                    }
                    p = p.Next;
                }
                Debug.Assert(false); // The request has to be queued if it timed out and we're not initialized yet.
            }
        }
        _connection.asyncRequestCanceled(outAsync, ex);
    }

    public Ice.ConnectionI getConnection()
    {
        lock (_mutex)
        {
            //
            // First check for the connection, it's important otherwise the user could first get a connection
            // and then the exception if he tries to obtain the proxy cached connection multiple times (the
            // exception can be set after the connection is set if the flush of pending requests fails).
            //
            if (_connection != null)
            {
                return _connection;
            }
            else if (_exception != null)
            {
                throw _exception;
            }
            return null;
        }
    }

    //
    // Implementation of Reference.GetConnectionCallback
    //

    public void setConnection(Ice.ConnectionI connection, bool compress)
    {
        lock (_mutex)
        {
            Debug.Assert(!_flushing && _exception == null && _connection == null);
            _connection = connection;
            _compress = compress;
        }

        //
        // If this proxy is for a non-local object, and we are using a router, then
        // add this proxy to the router info object.
        //
        RouterInfo ri = _reference.getRouterInfo();
        if (ri != null && !ri.addProxy(_reference, this))
        {
            return; // The request handler will be initialized once addProxy returns.
        }

        //
        // We can now send the queued requests.
        //
        flushRequests();
    }

    public void setException(Ice.LocalException ex)
    {
        lock (_mutex)
        {
            Debug.Assert(!_flushing && !_initialized && _exception == null);
            _exception = ex;
            _flushing = true; // Ensures request handler is removed before processing new requests.
        }

        foreach (ProxyOutgoingAsyncBase outAsync in _requests)
        {
            if (outAsync.exception(_exception))
            {
                outAsync.invokeExceptionAsync();
            }
        }
        _requests.Clear();

        lock (_mutex)
        {
            _flushing = false;
            Monitor.PulseAll(_mutex);
        }
    }

    //
    // Implementation of RouterInfo.AddProxyCallback
    //
    public void addedProxy()
    {
        //
        // The proxy was added to the router info, we're now ready to send the
        // queued requests.
        //
        flushRequests();
    }

    private bool initialized()
    {
        if (_initialized)
        {
            Debug.Assert(_connection != null);
            return true;
        }
        else
        {
            while (_flushing)
            {
                Monitor.Wait(_mutex);
            }

            if (_exception != null)
            {
                if (_connection != null)
                {
                    //
                    // Only throw if the connection didn't get established. If
                    // it died after being established, we allow the caller to
                    // retry the connection establishment by not throwing here
                    // (the connection will throw RetryException).
                    //
                    return true;
                }
                throw _exception;
            }
            else
            {
                return _initialized;
            }
        }
    }

    private void flushRequests()
    {
        lock (_mutex)
        {
            Debug.Assert(_connection != null && !_initialized);

            //
            // We set the _flushing flag to true to prevent any additional queuing. Callers
            // might block for a little while as the queued requests are being sent but this
            // shouldn't be an issue as the request sends are non-blocking.
            //
            _flushing = true;
        }

        Ice.LocalException exception = null;
        foreach (ProxyOutgoingAsyncBase outAsync in _requests)
        {
            try
            {
                if ((outAsync.invokeRemote(_connection, _compress, _response) & OutgoingAsyncBase.AsyncStatusInvokeSentCallback) != 0)
                {
                    outAsync.invokeSentAsync();
                }
            }
            catch (RetryException ex)
            {
                exception = ex.get();
                outAsync.retryException();
            }
            catch (Ice.LocalException ex)
            {
                exception = ex;
                if (outAsync.exception(ex))
                {
                    outAsync.invokeExceptionAsync();
                }
            }
        }
        _requests.Clear();

        lock (_mutex)
        {
            Debug.Assert(!_initialized);
            _exception = exception;
            _initialized = _exception == null;
            _flushing = false;
            Monitor.PulseAll(_mutex);
        }
    }

    private readonly Reference _reference;
    private readonly bool _response;

    private Ice.ConnectionI _connection;
    private bool _compress;
    private Ice.LocalException _exception;
    private bool _initialized;
    private bool _flushing;

    private readonly LinkedList<ProxyOutgoingAsyncBase> _requests = new();
    private readonly object _mutex = new();
}
