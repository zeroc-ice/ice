//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

namespace IceInternal
{
    public class ConnectRequestHandler : IRequestHandler, Reference.IGetConnectionCallback, RouterInfo.AddProxyCallback
    {
        public IRequestHandler Connect(Ice.IObjectPrx proxy)
        {
            lock (this)
            {
                if (!Initialized())
                {
                    _proxies.Add(proxy);
                }
                return _requestHandler;
            }
        }

        public IRequestHandler? Update(IRequestHandler? previousHandler, IRequestHandler? newHandler) =>
            previousHandler == this ? newHandler : this;

        public int SendAsyncRequest(ProxyOutgoingAsyncBase outAsync)
        {
            lock (this)
            {
                if (!_initialized)
                {
                    outAsync.Cancelable(this); // This will throw if the request is canceled
                }

                if (!Initialized())
                {
                    _requests.AddLast(outAsync);
                    return OutgoingAsyncBase.AsyncStatusQueued;
                }
            }
            Debug.Assert(_connection != null);
            return outAsync.InvokeRemote(_connection, _compress, !outAsync.IsOneway);
        }

        public void AsyncRequestCanceled(OutgoingAsyncBase outAsync, System.Exception ex)
        {
            lock (this)
            {
                if (_exception != null)
                {
                    return; // The request has been notified of a failure already.
                }

                if (!Initialized())
                {
                    LinkedListNode<ProxyOutgoingAsyncBase> p = _requests.First;
                    while (p != null)
                    {
                        if (p.Value == outAsync)
                        {
                            _requests.Remove(p);
                            if (outAsync.Exception(ex))
                            {
                                outAsync.InvokeExceptionAsync();
                            }
                            return;
                        }
                        p = p.Next;
                    }
                    Debug.Assert(false); // The request has to be queued if it timed out and we're not initialized yet.
                }
            }
            Debug.Assert(_connection != null);
            _connection.AsyncRequestCanceled(outAsync, ex);
        }

        public Reference GetReference() => _reference;

        public Ice.Connection? GetConnection()
        {
            lock (this)
            {
                //
                // First check for the connection, it's important otherwise the user could first get a connection
                // and then the exception if he tries to obtain the proxy cached connection mutiple times (the
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

        public void SetConnection(Ice.Connection connection, bool compress)
        {
            lock (this)
            {
                Debug.Assert(!_flushing && _exception == null && _connection == null);
                _connection = connection;
                _compress = compress;
            }

            //
            // If this proxy is for a non-local object, and we are using a router, then
            // add this proxy to the router info object.
            //
            Debug.Assert(_proxy != null);
            RouterInfo? ri = _reference.GetRouterInfo();
            if (ri != null && !ri.AddProxy(_proxy, this))
            {
                return; // The request handler will be initialized once addProxy returns.
            }

            //
            // We can now send the queued requests.
            //
            FlushRequests();
        }

        public void SetException(System.Exception ex)
        {
            lock (this)
            {
                Debug.Assert(!_flushing && !_initialized && _exception == null);
                _exception = ex;
                _flushing = true; // Ensures request handler is removed before processing new requests.
            }

            //
            // NOTE: remove the request handler *before* notifying the requests that the connection
            // failed. It's important to ensure that future invocations will obtain a new connect
            // request handler once invocations are notified.
            //
            try
            {
                _reference.GetCommunicator().RemoveRequestHandler(_reference, this);
            }
            catch (Ice.CommunicatorDestroyedException)
            {
                // Ignore
            }

            foreach (ProxyOutgoingAsyncBase outAsync in _requests)
            {
                if (outAsync.Exception(_exception))
                {
                    outAsync.InvokeExceptionAsync();
                }
            }
            _requests.Clear();

            lock (this)
            {
                _flushing = false;
                _proxies.Clear();
                _proxy = null; // Break cyclic reference count.
                Monitor.PulseAll(this);
            }
        }

        //
        // Implementation of RouterInfo.AddProxyCallback
        //
        // The proxy was added to the router info, we're now ready to send the
        // queued requests.
        //
        public void addedProxy() => FlushRequests();

        public ConnectRequestHandler(Reference @ref, Ice.IObjectPrx proxy)
        {
            _reference = @ref;
            _proxy = proxy;
            _initialized = false;
            _flushing = false;
            _requestHandler = this;
        }

        private bool Initialized()
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
                    Monitor.Wait(this);
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

        private void FlushRequests()
        {
            lock (this)
            {
                Debug.Assert(_connection != null && !_initialized);

                //
                // We set the _flushing flag to true to prevent any additional queuing. Callers
                // might block for a little while as the queued requests are being sent but this
                // shouldn't be an issue as the request sends are non-blocking.
                //
                _flushing = true;
            }

            System.Exception? exception = null;
            foreach (ProxyOutgoingAsyncBase outAsync in _requests)
            {
                try
                {
                    if ((outAsync.InvokeRemote(_connection, _compress, !outAsync.IsOneway) & OutgoingAsyncBase.AsyncStatusInvokeSentCallback) != 0)
                    {
                        outAsync.InvokeSentAsync();
                    }
                }
                catch (RetryException ex)
                {
                    exception = ex.Get();

                    // Remove the request handler before retrying.
                    _reference.GetCommunicator().RemoveRequestHandler(_reference, this);

                    outAsync.RetryException();
                }
                catch (System.Exception ex)
                {
                    exception = ex;
                    if (outAsync.Exception(ex))
                    {
                        outAsync.InvokeExceptionAsync();
                    }
                }
            }
            _requests.Clear();

            //
            // If we aren't caching the connection, don't bother creating a
            // connection request handler. Otherwise, update the proxies
            // request handler to use the more efficient connection request
            // handler.
            //
            if (_reference.GetCacheConnection() && exception == null)
            {
                _requestHandler = new ConnectionRequestHandler(_reference, _connection, _compress);
                foreach (Ice.IObjectPrx prx in _proxies)
                {
                    prx.IceUpdateRequestHandler(this, _requestHandler);
                }
            }

            lock (this)
            {
                Debug.Assert(!_initialized);
                _exception = exception;
                _initialized = _exception == null;
                _flushing = false;

                //
                // Only remove once all the requests are flushed to
                // guarantee serialization.
                //
                _reference.GetCommunicator().RemoveRequestHandler(_reference, this);

                _proxies.Clear();
                _proxy = null; // Break cyclic reference count.
                Monitor.PulseAll(this);
            }
        }

        private readonly Reference _reference;

        private IObjectPrx? _proxy;
        private readonly HashSet<IObjectPrx> _proxies = new HashSet<IObjectPrx>();

        private Connection? _connection;
        private bool _compress;
        private System.Exception? _exception;
        private bool _initialized;
        private bool _flushing;

        private readonly LinkedList<ProxyOutgoingAsyncBase> _requests = new LinkedList<ProxyOutgoingAsyncBase>();
        private IRequestHandler _requestHandler;
    }
}
