// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

final class ConnectRequestHandler
        implements RequestHandler, Reference.GetConnectionCallback, RouterInfo.AddProxyCallback {
    @Override
    public int sendAsyncRequest(ProxyOutgoingAsyncBase out) throws RetryException {
        synchronized (this) {
            if (!_initialized) {
                out.cancelable(this); // This will throw if the request is canceled
            }

            if (!initialized()) {
                _requests.add(out);
                return AsyncStatus.Queued;
            }
        }
        return out.invokeRemote(_connection, _compress, _response);
    }

    @Override
    public void asyncRequestCanceled(OutgoingAsyncBase outAsync, LocalException ex) {
        synchronized (this) {
            if (_exception != null) {
                return; // The request has been notified of a failure already.
            }

            if (!initialized()) {
                java.util.Iterator<ProxyOutgoingAsyncBase> it = _requests.iterator();
                while (it.hasNext()) {
                    OutgoingAsyncBase request = it.next();
                    if (request == outAsync) {
                        it.remove();
                        if (outAsync.completed(ex)) {
                            outAsync.invokeCompletedAsync();
                        }
                        return;
                    }
                }
                // The request has to be queued if it timed out and we're not initialized yet.
                assert (false);
            }
        }
        _connection.asyncRequestCanceled(outAsync, ex);
    }

    @Override
    public synchronized ConnectionI getConnection() {
        //
        // First check for the connection, it's important otherwise the user could first get a
        // connection and then the exception if he tries to obtain the proxy cached connection
        // multiple times (the exception can be set after the connection is set if the flush of
        // pending requests fails).
        //
        if (_connection != null) {
            return _connection;
        } else if (_exception != null) {
            throw (LocalException) _exception.fillInStackTrace();
        }
        return null;
    }

    //
    // Implementation of Reference.GetConnectionCallback
    //

    @Override
    public void setConnection(ConnectionI connection, boolean compress) {
        synchronized (this) {
            assert (!_flushing && _exception == null && _connection == null);
            _connection = connection;
            _compress = compress;
        }

        //
        // If this proxy is for a non-local object, and we are using a router, then add this proxy
        // to the router info object.
        //
        RouterInfo ri = _reference.getRouterInfo();
        if (ri != null && !ri.addProxy(_reference, this)) {
            return; // The request handler will be initialized once addProxy returns.
        }

        //
        // We can now send the queued requests.
        //
        flushRequests();
    }

    @Override
    public void setException(final LocalException ex) {
        synchronized (this) {
            assert (!_flushing && !_initialized && _exception == null);
            _exception = ex;
            _flushing = true; // Ensures request handler is removed before processing new requests.
        }

        for (OutgoingAsyncBase outAsync : _requests) {
            if (outAsync.completed(_exception)) {
                outAsync.invokeCompletedAsync();
            }
        }
        _requests.clear();

        synchronized (this) {
            _flushing = false;
            notifyAll();
        }
    }

    //
    // Implementation of RouterInfo.AddProxyCallback
    //
    @Override
    public void addedProxy() {
        //
        // The proxy was added to the router info, we're now ready to send the queued requests.
        //
        flushRequests();
    }

    public ConnectRequestHandler(Reference ref) {
        _reference = ref;
        _response = _reference.isTwoway();
        _initialized = false;
        _flushing = false;
    }

    private boolean initialized() {
        // Must be called with the mutex locked.

        if (_initialized) {
            assert (_connection != null);
            return true;
        } else {
            //
            // This is similar to a mutex lock in that the flag is only true for a short period of
            // time.
            //
            boolean interrupted = false;
            while (_flushing) {
                try {
                    wait();
                } catch (InterruptedException ex) {
                    interrupted = true;
                }
            }
            //
            // Restore the interrupted status.
            //
            if (interrupted) {
                Thread.currentThread().interrupt();
            }

            if (_exception != null) {
                if (_connection != null) {
                    //
                    // Only throw if the connection didn't get established. If it died after being
                    // established, we allow the caller to retry the connection establishment by not
                    // throwing here
                    // (the connection will throw RetryException).
                    //
                    return true;
                }
                throw (LocalException) _exception.fillInStackTrace();
            } else {
                return _initialized;
            }
        }
    }

    private void flushRequests() {
        synchronized (this) {
            assert (_connection != null && !_initialized);

            //
            // We set the _flushing flag to true to prevent any additional queuing. Callers might
            // block for a little while as the queued requests are being sent but this shouldn't be
            // an issue as the request sends are non-blocking.
            //
            _flushing = true;
        }

        LocalException exception = null;
        for (ProxyOutgoingAsyncBase outAsync : _requests) {
            try {
                if ((outAsync.invokeRemote(_connection, _compress, _response)
                                & AsyncStatus.InvokeSentCallback)
                        > 0) {
                    outAsync.invokeSentAsync();
                }
            } catch (RetryException ex) {
                exception = ex.get();
                outAsync.retryException();
            } catch (LocalException ex) {
                exception = ex;
                if (outAsync.completed(ex)) {
                    outAsync.invokeCompletedAsync();
                }
            }
        }
        _requests.clear();

        synchronized (this) {
            assert (!_initialized);
            _exception = exception;
            _initialized = _exception == null;
            _flushing = false;
            notifyAll();
        }
    }

    private final Reference _reference;
    private boolean _response;

    private ConnectionI _connection;
    private boolean _compress;
    private LocalException _exception;
    private boolean _initialized;
    private boolean _flushing;

    private java.util.List<ProxyOutgoingAsyncBase> _requests = new java.util.LinkedList<>();
}
