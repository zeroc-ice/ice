// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
        private class Request
        {
            internal Request(BasicStream os)
            {
                this.os = new BasicStream(os.instance(), Ice.Util.currentProtocolEncoding);
                this.os.swap(os);
            }

            internal Request(OutgoingAsyncBase outAsync)
            {
                this.outAsync = outAsync;
            }

            internal OutgoingAsyncBase outAsync = null;
            internal BasicStream os = null;
            internal Ice.AsyncCallback sentCallback = null;
        }

        public RequestHandler connect(Ice.ObjectPrxHelperBase proxy)
        {
            //
            // Initiate the connection if connect() is called by the proxy that
            // created the handler.
            //
            if(Object.ReferenceEquals(proxy, _proxy) && _connect)
            {
                _connect = false; // Call getConnection only once
                _reference.getConnection(this);
            }

            try
            {
                lock(this)
                {
                    if(!initialized())
                    {
                        _proxies.Add(proxy);
                        return this;
                    }
                }
            }
            catch(Ice.LocalException ex)
            {
                throw ex;
            }

            if(_connectionRequestHandler != null)
            {
                proxy.setRequestHandler__(this, _connectionRequestHandler);
                return _connectionRequestHandler;
            }
            else
            {
                return this;                
            }
        }

        public RequestHandler update(RequestHandler previousHandler, RequestHandler newHandler)
        {
            return previousHandler == this ? newHandler : this;
        }

        public void prepareBatchRequest(BasicStream os)
        {
            lock(this)
            {
                while(_batchRequestInProgress)
                {
                    System.Threading.Monitor.Wait(this);
                }

                try
                {
                    if(!initialized())
                    {
                        _batchRequestInProgress = true;
                        _batchStream.swap(os);
                        return;
                    }
                }
                catch(Ice.LocalException ex)
                {
                    throw new RetryException(ex);
                }
            }
            _connection.prepareBatchRequest(os);
        }

        public void finishBatchRequest(BasicStream os)
        {
            lock(this)
            {
                if(!initialized()) // This can't throw until _batchRequestInProgress = false
                {
                    Debug.Assert(_batchRequestInProgress);
                    _batchRequestInProgress = false;
                    System.Threading.Monitor.PulseAll(this);

                    _batchStream.swap(os);

                    if(!_batchAutoFlush &&
                       _batchStream.size() + _batchRequestsSize > _reference.getInstance().messageSizeMax())
                    {
                        Ex.throwMemoryLimitException(_batchStream.size() + _batchRequestsSize,
                                                     _reference.getInstance().messageSizeMax());
                    }
                    _requests.AddLast(new Request(_batchStream));
                    return;
                }
            }
            _connection.finishBatchRequest(os, _compress);
        }

        public void abortBatchRequest()
        {
            lock(this)
            {
                if(!initialized()) // This can't throw until _batchRequestInProgress = false
                {
                    Debug.Assert(_batchRequestInProgress);
                    _batchRequestInProgress = false;
                    System.Threading.Monitor.PulseAll(this);

                    BasicStream dummy = new BasicStream(_reference.getInstance(), Ice.Util.currentProtocolEncoding,
                                                        _batchAutoFlush);
                    _batchStream.swap(dummy);
                    _batchRequestsSize = Protocol.requestBatchHdr.Length;

                    return;
                }
            }
            _connection.abortBatchRequest();
        }

        public bool sendAsyncRequest(OutgoingAsyncBase outAsync, out Ice.AsyncCallback sentCallback)
        {
            lock(this)
            {
                if(!_initialized)
                {
                    outAsync.cancelable(this); // This will throw if the request is canceled
                }

                try
                {
                    if(!initialized())
                    {
                        _requests.AddLast(new Request(outAsync));
                        sentCallback = null;
                        return false;
                    }
                }
                catch(Ice.LocalException ex)
                {
                    throw new RetryException(ex);
                }
            }
            return outAsync.send(_connection, _compress, _response, out sentCallback);
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
                    LinkedListNode<Request> p = _requests.First;
                    while(p != null)
                    {
                        Request request = p.Value;
                        if(request.outAsync == outAsync)
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
                if(_exception != null)
                {
                    throw _exception;
                }
                else
                {
                    return _connection;
                }
            }
        }

        public Ice.ConnectionI waitForConnection()
        {
            lock(this)
            {
                if(_exception != null)
                {
                    throw new RetryException(_exception);
                }
                //
                // Wait for the connection establishment to complete or fail.
                //
                while(!_initialized && _exception == null)
                {
                    System.Threading.Monitor.Wait(this);
                }
                return getConnection();
            }
        }

        //
        // Implementation of Reference.GetConnectionCallback
        //

        public void setConnection(Ice.ConnectionI connection, bool compress)
        {
            lock(this)
            {
                Debug.Assert(_exception == null && _connection == null);
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
                Debug.Assert(!_initialized && _exception == null);
                _exception = ex;
                _proxies.Clear();
                _proxy = null; // Break cyclic reference count.

                //
                // NOTE: remove the request handler *before* notifying the
                // requests that the connection failed. It's important to ensure
                // that future invocations will obtain a new connect request
                // handler once invocations are notified.
                //
                try
                {
                    _reference.getInstance().requestHandlerFactory().removeRequestHandler(_reference, this);
                }
                catch(Ice.CommunicatorDestroyedException)
                {
                    // Ignore
                }

                flushRequestsWithException();
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
            _connect = true;
            _response = _reference.getMode() == Reference.Mode.ModeTwoway;
            _proxy = (Ice.ObjectPrxHelperBase)proxy;
            _batchAutoFlush = @ref.getInstance().initializationData().properties.getPropertyAsIntWithDefault(
                "Ice.BatchAutoFlush", 1) > 0 ? true : false;
            _initialized = false;
            _flushing = false;
            _batchRequestInProgress = false;
            _batchRequestsSize = Protocol.requestBatchHdr.Length;
            _batchStream = new BasicStream(@ref.getInstance(), Ice.Util.currentProtocolEncoding, _batchAutoFlush);
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
                while(_flushing && _exception == null)
                {
                    System.Threading.Monitor.Wait(this);
                }

                if(_exception != null)
                {
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

                while(_batchRequestInProgress)
                {
                    System.Threading.Monitor.Wait(this);
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
                LinkedListNode<Request> p = _requests.First; // _requests is immutable when _flushing = true
                while(p != null)
                {
                    Request request = p.Value;
                    if(request.outAsync != null)
                    {
                        if(request.outAsync.send(_connection, _compress, _response, out request.sentCallback))
                        {
                            if(request.sentCallback != null)
                            {
                                request.outAsync.invokeSentAsync(request.sentCallback);
                            }
                        }
                    }
                    else
                    {
                        BasicStream os = new BasicStream(request.os.instance(), Ice.Util.currentProtocolEncoding);
                        _connection.prepareBatchRequest(os);
                        try
                        {
                            request.os.pos(0);
                            os.writeBlob(request.os.readBlob(request.os.size()));
                        }
                        catch(Ice.LocalException)
                        {
                            _connection.abortBatchRequest();
                            throw;
                        }
                        _connection.finishBatchRequest(os, _compress);
                    }
                    LinkedListNode<Request> tmp = p;
                    p = p.Next;
                    _requests.Remove(tmp);
                }
            }
            catch(RetryException ex)
            {
                //
                // If the connection dies shortly after connection
                // establishment, we don't systematically retry on
                // RetryException. We handle the exception like it
                // was an exception that occured while sending the
                // request.
                //
                lock(this)
                {
                    Debug.Assert(_exception == null && _requests.Count > 0);
                    _exception = ex.get();
                    flushRequestsWithException();
                }
            }
            catch(Ice.LocalException ex)
            {
                lock(this)
                {
                    Debug.Assert(_exception == null && _requests.Count > 0);
                    _exception = ex;
                    flushRequestsWithException();
                }
            }

            //
            // If we aren't caching the connection, don't bother creating a
            // connection request handler. Otherwise, update the proxies
            // request handler to use the more efficient connection request
            // handler.
            //
            if(_reference.getCacheConnection() && _exception == null)
            {
                _connectionRequestHandler = new ConnectionRequestHandler(_reference, _connection, _compress);
                foreach(Ice.ObjectPrxHelperBase prx in _proxies)
                {
                    prx.setRequestHandler__(this, _connectionRequestHandler);
                }
            }

            lock(this)
            {
                Debug.Assert(!_initialized);
                if(_exception == null)
                {
                    _initialized = true;
                    _flushing = false;
                }
                try
                {
                    _reference.getInstance().requestHandlerFactory().removeRequestHandler(_reference, this);
                }
                catch(Ice.CommunicatorDestroyedException)
                {
                    // Ignore
                }
                _proxies.Clear();
                _proxy = null; // Break cyclic reference count.
                System.Threading.Monitor.PulseAll(this);
            }
        }

        private void
        flushRequestsWithException()
        {
            foreach(Request request in _requests)
            {
                if(request.outAsync != null)
                {
                    Ice.AsyncCallback cb = request.outAsync.completed(_exception);
                    if(cb != null)
                    {
                        request.outAsync.invokeCompletedAsync(cb);
                    }
                }
            }
            _requests.Clear();
        }

        private Reference _reference;
        private bool _connect;
        private bool _response;

        private Ice.ObjectPrxHelperBase _proxy;
        private HashSet<Ice.ObjectPrxHelperBase> _proxies = new HashSet<Ice.ObjectPrxHelperBase>();

        private bool _batchAutoFlush;

        private Ice.ConnectionI _connection;
        private bool _compress;
        private Ice.LocalException _exception;
        private bool _initialized;
        private bool _flushing;

        private LinkedList<Request> _requests = new LinkedList<Request>();
        private bool _batchRequestInProgress;
        private int _batchRequestsSize;
        private BasicStream _batchStream;

        private RequestHandler _connectionRequestHandler;
    }
}
