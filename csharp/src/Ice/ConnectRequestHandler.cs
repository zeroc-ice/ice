// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using Ice.Instrumentation;

namespace IceInternal
{
    public class ConnectRequestHandler : RequestHandler, Reference.GetConnectionCallback, RouterInfo.AddProxyCallback
    {
        public RequestHandler connect(Ice.ObjectPrxHelperBase proxy)
        {
            lock(this)
            {
                if(!initialized())
                {
                    _proxies.Add(proxy);
                }
                return _requestHandler;
            }
        }

        public RequestHandler update(RequestHandler previousHandler, RequestHandler newHandler)
        {
            return previousHandler == this ? newHandler : this;
        }

        public bool sendAsyncRequest(ProxyOutgoingAsyncBase outAsync, out Ice.AsyncCallback sentCallback)
        {
            lock(this)
            {
                if(!_initialized)
                {
                    outAsync.cancelable(this); // This will throw if the request is canceled
                }

                if(!initialized())
                {
                    _requests.AddLast(outAsync);
                    sentCallback = null;
                    return false;
                }
            }
            return outAsync.invokeRemote(_connection, _compress, _response, out sentCallback);
        }

        public void asyncRequestCanceled(OutgoingAsyncBase outAsync, Ice.LocalException ex)
        {
            lock(this)
            {
                if(_exception != null)
                {
                    return; // The request has been notified of a failure already.
                }

                if(!initialized())
                {
                    LinkedListNode<ProxyOutgoingAsyncBase> p = _requests.First;
                    while(p != null)
                    {
                        if(p.Value == outAsync)
                        {
                            _requests.Remove(p);
                            Ice.AsyncCallback cb = outAsync.completed(ex);
                            if(cb != null)
                            {
                                outAsync.invokeCompletedAsync(cb);
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

        public Reference getReference()
        {
            return _reference;
        }

        public Ice.ConnectionI getConnection()
        {
            lock(this)
            {
                //
                // First check for the connection, it's important otherwise the user could first get a connection
                // and then the exception if he tries to obtain the proxy cached connection mutiple times (the
                // exception can be set after the connection is set if the flush of pending requests fails).
                //
                if(_connection != null)
                {
                    return _connection;
                }
                else if(_exception != null)
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
            lock(this)
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
            if(ri != null && !ri.addProxy(_proxy, this))
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
            lock(this)
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
                _reference.getInstance().requestHandlerFactory().removeRequestHandler(_reference, this);
            }
            catch(Ice.CommunicatorDestroyedException)
            {
                // Ignore
            }

            foreach(ProxyOutgoingAsyncBase outAsync in _requests)
            {
                Ice.AsyncCallback cb = outAsync.completed(_exception);
                if(cb != null)
                {
                    outAsync.invokeCompletedAsync(cb);
                }
            }
            _requests.Clear();

            lock(this)
            {
                _flushing = false;
                _proxies.Clear();
                _proxy = null; // Break cyclic reference count.
                System.Threading.Monitor.PulseAll(this);
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

        public ConnectRequestHandler(Reference @ref, Ice.ObjectPrx proxy)
        {
            _reference = @ref;
            _response = _reference.getMode() == Reference.Mode.ModeTwoway;
            _proxy = (Ice.ObjectPrxHelperBase)proxy;
            _initialized = false;
            _flushing = false;
            _requestHandler = this;
        }

        private bool initialized()
        {
            if(_initialized)
            {
                Debug.Assert(_connection != null);
                return true;
            }
            else
            {
                while(_flushing)
                {
                    System.Threading.Monitor.Wait(this);
                }

                if(_exception != null)
                {
                    if(_connection != null)
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
            lock(this)
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
            foreach(ProxyOutgoingAsyncBase outAsync in _requests)
            {
                try
                {
                    Ice.AsyncCallback sentCallback = null;
                    if(outAsync.invokeRemote(_connection, _compress, _response, out sentCallback))
                    {
                        if(sentCallback != null)
                        {
                            outAsync.invokeSentAsync(sentCallback);
                        }
                    }
                }
                catch(RetryException ex)
                {
                    exception = ex.get();

                    // Remove the request handler before retrying.
                    _reference.getInstance().requestHandlerFactory().removeRequestHandler(_reference, this);

                    outAsync.retryException(ex.get());
                }
                catch(Ice.LocalException ex)
                {
                    exception = ex;
                    Ice.AsyncCallback cb = outAsync.completed(ex);
                    if(cb != null)
                    {
                        outAsync.invokeCompletedAsync(cb);
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
            if(_reference.getCacheConnection() && exception == null)
            {
                _requestHandler = new ConnectionRequestHandler(_reference, _connection, _compress);
                foreach(Ice.ObjectPrxHelperBase prx in _proxies)
                {
                    prx.updateRequestHandler__(this, _requestHandler);
                }
            }

            lock(this)
            {
                Debug.Assert(!_initialized);
                _exception = exception;
                _initialized = _exception == null;
                _flushing = false;

                //
                // Only remove once all the requests are flushed to
                // guarantee serialization.
                //
                _reference.getInstance().requestHandlerFactory().removeRequestHandler(_reference, this);

                _proxies.Clear();
                _proxy = null; // Break cyclic reference count.
                System.Threading.Monitor.PulseAll(this);
            }
        }

        private Reference _reference;
        private bool _response;

        private Ice.ObjectPrxHelperBase _proxy;
        private HashSet<Ice.ObjectPrxHelperBase> _proxies = new HashSet<Ice.ObjectPrxHelperBase>();

        private Ice.ConnectionI _connection;
        private bool _compress;
        private Ice.LocalException _exception;
        private bool _initialized;
        private bool _flushing;

        private LinkedList<ProxyOutgoingAsyncBase> _requests = new LinkedList<ProxyOutgoingAsyncBase>();
        private RequestHandler _requestHandler;
    }
}
