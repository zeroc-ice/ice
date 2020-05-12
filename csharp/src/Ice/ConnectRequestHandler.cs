//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace IceInternal
{
    public class ConnectRequestHandler : IRequestHandler, Reference.IGetConnectionCallback, RouterInfo.IAddProxyCallback
    {
        public IRequestHandler Connect(Reference reference)
        {
            Debug.Assert(reference == _reference); // not ReferenceEquals, see Communicator.GetRequestHandler
            lock (this)
            {
                if (!Initialized())
                {
                    if (_reference.IsConnectionCached)
                    {
                        _referenceList ??= new List<Reference>();
                        _referenceList.Add(reference);
                    }
                }
                return _requestHandler;
            }
        }

        public IRequestHandler? Update(IRequestHandler? previousHandler, IRequestHandler? newHandler) =>
            previousHandler == this ? newHandler : this;

        public void SendAsyncRequest(ProxyOutgoingAsyncBase outAsync)
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
                    return;
                }
            }
            Debug.Assert(_connection != null);
            outAsync.InvokeRemote(_connection, _compress, !outAsync.IsOneway);
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
                    LinkedListNode<ProxyOutgoingAsyncBase>? p = _requests.First;
                    while (p != null)
                    {
                        if (p.Value == outAsync)
                        {
                            _requests.Remove(p);
                            if (outAsync.Exception(ex))
                            {
                                Task.Run(outAsync.InvokeException);
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

        public Ice.Connection? GetConnection()
        {
            lock (this)
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
            RouterInfo? ri = _reference.RouterInfo;
            if (ri != null && !ri.AddProxy(IObjectPrx.Factory(_reference), this))
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
                _reference.Communicator.RemoveConnectRequestHandler(_reference, this);
            }
            catch (Ice.CommunicatorDestroyedException)
            {
                // Ignore
            }

            foreach (ProxyOutgoingAsyncBase outAsync in _requests)
            {
                if (outAsync.Exception(_exception))
                {
                    Task.Run(outAsync.InvokeException);
                }
            }
            _requests.Clear();

            lock (this)
            {
                _flushing = false;
                _referenceList = null;
                Monitor.PulseAll(this);
            }
        }

        //
        // Implementation of RouterInfo.AddProxyCallback
        //
        // The proxy was added to the router info, we're now ready to send the
        // queued requests.
        //
        public void AddedProxy() => FlushRequests();

        public ConnectRequestHandler(Reference reference)
        {
            Debug.Assert(!reference.IsFixed);
            _reference = reference;
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
                    outAsync.InvokeRemote(_connection, _compress, !outAsync.IsOneway);
                }
                catch (RetryException ex)
                {
                    exception = ex.InnerException;

                    // Remove the request handler before retrying.
                    _reference.Communicator.RemoveConnectRequestHandler(_reference, this);

                    outAsync.RetryException();
                }
                catch (System.Exception ex)
                {
                    exception = ex;
                    if (outAsync.Exception(ex))
                    {
                        Task.Run(outAsync.InvokeException);
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
            if (_reference.IsConnectionCached && exception == null)
            {
                _requestHandler = new ConnectionRequestHandler(_connection, _compress);
                if (_referenceList != null)
                {
                    foreach (var reference in _referenceList)
                    {
                        reference.UpdateRequestHandler(this, _requestHandler);
                    }
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
                _reference.Communicator.RemoveConnectRequestHandler(_reference, this);

                _referenceList = null;
                Monitor.PulseAll(this);
            }
        }

        private readonly Reference _reference;

        // References to update upon initialization
        private List<Reference>? _referenceList;

        private Connection? _connection;
        private bool _compress;
        private System.Exception? _exception;
        private bool _initialized;
        private bool _flushing;

        private readonly LinkedList<ProxyOutgoingAsyncBase> _requests = new LinkedList<ProxyOutgoingAsyncBase>();
        private IRequestHandler _requestHandler;
    }
}
