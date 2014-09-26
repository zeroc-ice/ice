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

            internal Request(OutgoingAsyncMessageCallback outAsync)
            {
                this.outAsync = outAsync;
            }

            internal OutgoingAsyncMessageCallback outAsync = null;
            internal BasicStream os = null;
            internal Ice.AsyncCallback sentCallback = null;
        }

        public RequestHandler connect()
        {
            Ice.ObjectPrxHelperBase proxy = _proxy;

            _reference.getConnection(this);

            try
            {
                lock(this)
                {
                    if(!initialized())
                    {
                        // The proxy request handler will be updated when the connection is set.
                        _updateRequestHandler = true;
                        return this;
                    }
                }
            }
            catch(Ice.LocalException ex)
            {
                proxy.setRequestHandler__(this, null);
                throw ex;
            }

            Debug.Assert(_connection != null);

            RequestHandler handler = new ConnectionRequestHandler(_reference, _connection, _compress);
            proxy.setRequestHandler__(this, handler);
            return handler;
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

        public bool sendAsyncRequest(OutgoingAsyncMessageCallback outAsync, out Ice.AsyncCallback sentCallback)
        {
            lock(this)
            {
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

        public void asyncRequestCanceled(OutgoingAsyncMessageCallback outAsync, Ice.LocalException ex)
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
                            outAsync.dispatchInvocationCancel(ex, _reference.getInstance().clientThreadPool(), null);
                            return; // We're done
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
                _proxy = null; // Break cyclic reference count.

                //
                // If some requests were queued, we notify them of the failure. This is done from a thread
                // from the client thread pool since this will result in ice_exception callbacks to be
                // called.
                //
                if(_requests.Count > 0)
                {
                    _reference.getInstance().clientThreadPool().dispatch(() =>
                    {
                        flushRequestsWithException();
                    }, _connection);
                }

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
            _batchAutoFlush = @ref.getInstance().initializationData().properties.getPropertyAsIntWithDefault(
                "Ice.BatchAutoFlush", 1) > 0 ? true : false;
            _initialized = false;
            _flushing = false;
            _batchRequestInProgress = false;
            _batchRequestsSize = Protocol.requestBatchHdr.Length;
            _batchStream = new BasicStream(@ref.getInstance(), Ice.Util.currentProtocolEncoding, _batchAutoFlush);
            _updateRequestHandler = false;
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

            LinkedList<Request> sentCallbacks = new LinkedList<Request>();
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
                                sentCallbacks.AddLast(request);
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
                    _reference.getInstance().clientThreadPool().dispatch(() =>
                    {
                        flushRequestsWithException();
                    }, _connection);
                }
            }
            catch(Ice.LocalException ex)
            {
                lock(this)
                {
                    Debug.Assert(_exception == null && _requests.Count > 0);
                    _exception = ex;
                    _reference.getInstance().clientThreadPool().dispatch(() =>
                    {
                        flushRequestsWithException();
                    }, _connection);
                }
            }

            if(sentCallbacks.Count > 0)
            {
                Instance instance = _reference.getInstance();
                instance.clientThreadPool().dispatch(() =>
                {
                    foreach(Request r in sentCallbacks)
                    {
                        if(r.outAsync != null)
                        {
                            r.outAsync.invokeSent(r.sentCallback);
                        }
                    }
                }, _connection);
            }

            //
            // We've finished sending the queued requests and the request handler now send
            // the requests over the connection directly. It's time to substitute the
            // request handler of the proxy with the more efficient connection request
            // handler which does not have any synchronization. This also breaks the cyclic
            // reference count with the proxy.
            //
            // NOTE: _updateRequestHandler is immutable once _flushing = true
            //
            if(_updateRequestHandler && _exception == null)
            {
                _proxy.setRequestHandler__(this, new ConnectionRequestHandler(_reference, _connection, _compress));
            }

            lock(this)
            {
                Debug.Assert(!_initialized);
                if(_exception == null)
                {
                    _initialized = true;
                    _flushing = false;
                }
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
                    request.outAsync.finished(_exception);
                }
            }
            _requests.Clear();
        }

        private Reference _reference;
        private bool _response;

        private Ice.ObjectPrxHelperBase _proxy;

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
        private bool _updateRequestHandler;
    }
}
