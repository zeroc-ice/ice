// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

            internal Request(OutgoingAsync @out)
            {
                this.@out = @out;
            }

            internal Request(BatchOutgoingAsync @out)
            {
                this.batchOut = @out;
            }

            internal OutgoingAsync @out = null;
            internal BatchOutgoingAsync batchOut = null;
            internal BasicStream os = null;
            internal Ice.AsyncCallback sentCallback = null;
        }

        public RequestHandler connect()
        {
            _reference.getConnection(this);

            _m.Lock();
            try
            {
                if(initialized())
                {
                    Debug.Assert(_connection != null);
                    return new ConnectionRequestHandler(_reference, _connection, _compress);
                }
                else
                {
                    // The proxy request handler will be updated when the connection is set.
                    _updateRequestHandler = true;
                    return this;
                }
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void prepareBatchRequest(BasicStream os)
        {
            _m.Lock();
            try
            {
                while(_batchRequestInProgress)
                {
                    _m.Wait();
                }

                if(!initialized())
                {
                    _batchRequestInProgress = true;
                    _batchStream.swap(os);
                    return;
                }
            }
            finally
            {
                _m.Unlock();
            }
            _connection.prepareBatchRequest(os);
        }

        public void finishBatchRequest(BasicStream os)
        {
            _m.Lock();
            try
            {
                if(!initialized())
                {
                    Debug.Assert(_batchRequestInProgress);
                    _batchRequestInProgress = false;
                    _m.NotifyAll();

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
            finally
            {
                _m.Unlock();
            }
            _connection.finishBatchRequest(os, _compress);
        }

        public void abortBatchRequest()
        {
            _m.Lock();
            try
            {
                if(!initialized())
                {
                    Debug.Assert(_batchRequestInProgress);
                    _batchRequestInProgress = false;
                    _m.NotifyAll();

                    BasicStream dummy = new BasicStream(_reference.getInstance(), Ice.Util.currentProtocolEncoding, 
                                                        _batchAutoFlush);
                    _batchStream.swap(dummy);
                    _batchRequestsSize = Protocol.requestBatchHdr.Length;

                    return;
                }
            }
            finally
            {
                _m.Unlock();
            }
            _connection.abortBatchRequest();
        }

        public Ice.ConnectionI sendRequest(Outgoing @out)
        {
            Ice.ConnectionI connection = getConnection(true);
            Debug.Assert(connection != null);
            if(!connection.sendRequest(@out, _compress, _response) || _response)
            {
                return _connection; // The request has been sent or we're expecting a response.
            }
            else
            {
                return null; // The request hasn't been sent yet.
            }
        }

        public bool sendAsyncRequest(OutgoingAsync @out, out Ice.AsyncCallback sentCallback)
        {
            _m.Lock();
            try
            {
                if(!initialized())
                {
                    _requests.AddLast(new Request(@out));
                    sentCallback = null;
                    return false;
                }
            }
            finally
            {
                _m.Unlock();
            }
            return _connection.sendAsyncRequest(@out, _compress, _response, out sentCallback);
        }

        public bool flushBatchRequests(BatchOutgoing @out)
        {
            return getConnection(true).flushBatchRequests(@out);
        }

        public bool flushAsyncBatchRequests(BatchOutgoingAsync @out, out Ice.AsyncCallback sentCallback)
        {
            _m.Lock();
            try
            {
                if(!initialized())
                {
                    _requests.AddLast(new Request(@out));
                    sentCallback = null;
                    return false;
                }
            }
            finally
            {
                _m.Unlock();
            }
            return _connection.flushAsyncBatchRequests(@out, out sentCallback);
        }

        public Outgoing getOutgoing(string operation, Ice.OperationMode mode, Dictionary<string, string> context,
                                    InvocationObserver observer)
        {
            _m.Lock();
            try
            {
                if(!initialized())
                {
                    return new IceInternal.Outgoing(this, operation, mode, context, observer);
                }
            }
            finally
            {
                _m.Unlock();
            }

            return _connection.getOutgoing(this, operation, mode, context, observer);
        }

        public void reclaimOutgoing(Outgoing og)
        {
            _m.Lock();
            try
            {
                if(_connection == null)
                {
                    return;
                }
            }
            finally
            {
                _m.Unlock();
            }

            _connection.reclaimOutgoing(og);
        }

        public Reference getReference()
        {
            return _reference;
        }

        public Ice.ConnectionI getConnection(bool waitInit)
        {
            if(waitInit)
            {
                _m.Lock();
                try
                {
                    //
                    // Wait for the connection establishment to complete or fail.
                    //
                    while(!_initialized && _exception == null)
                    {
                        _m.Wait();
                    }
                }
                finally
                {
                    _m.Unlock();
                }
            }

            if(_exception != null)
            {
                throw _exception;
            }
            else
            {
                Debug.Assert(!waitInit || _initialized);
                return _connection;
            }
        }

        //
        // Implementation of Reference.GetConnectionCallback
        //

        public void setConnection(Ice.ConnectionI connection, bool compress)
        {
            _m.Lock();
            try
            {
                Debug.Assert(_exception == null && _connection == null);
                Debug.Assert(_updateRequestHandler || _requests.Count == 0);

                _connection = connection;
                _compress = compress;
            }
            finally
            {
                _m.Unlock();
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
            _m.Lock();
            try
            {
                Debug.Assert(!_initialized && _exception == null);
                Debug.Assert(_updateRequestHandler || _requests.Count == 0);

                _exception = ex;
                _proxy = null; // Break cyclic reference count.
                _delegate = null; // Break cyclic reference count.

                //
                // If some requests were queued, we notify them of the failure. This is done from a thread
                // from the client thread pool since this will result in ice_exception callbacks to be
                // called.
                //
                if(_requests.Count > 0)
                {
                    _reference.getInstance().clientThreadPool().dispatch(delegate()
                                                                        {
                                                                            flushRequestsWithException(ex);
                                                                        });
                }

                _m.NotifyAll();
            }
            finally
            {
                _m.Unlock();
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

        public ConnectRequestHandler(Reference @ref, Ice.ObjectPrx proxy, Ice.ObjectDelM_ del)
        {
            _reference = @ref;
            _response = _reference.getMode() == Reference.Mode.ModeTwoway;
            _proxy = (Ice.ObjectPrxHelperBase)proxy;
            _delegate = del;
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
                    _m.Wait();
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
            _m.Lock();
            try
            {
                Debug.Assert(_connection != null && !_initialized);

                while(_batchRequestInProgress)
                {
                    _m.Wait();
                }

                //
                // We set the _flushing flag to true to prevent any additional queuing. Callers
                // might block for a little while as the queued requests are being sent but this
                // shouldn't be an issue as the request sends are non-blocking.
                //
                _flushing = true;
            }
            finally
            {
                _m.Unlock();
            }

            LinkedList<Request> sentCallbacks = new LinkedList<Request>();
            try
            {
                LinkedListNode<Request> p = _requests.First; // _requests is immutable when _flushing = true
                while(p != null)
                {
                    Request request = p.Value;
                    if(request.@out != null)
                    {
                        if(_connection.sendAsyncRequest(request.@out, _compress, _response, out request.sentCallback))
                        {
                            if(request.sentCallback != null)
                            {
                                sentCallbacks.AddLast(request);
                            }
                        }
                    }
                    else if(request.batchOut != null)
                    {
                        if(_connection.flushAsyncBatchRequests(request.batchOut, out request.sentCallback))
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
            catch(LocalExceptionWrapper ex)
            {
                _m.Lock();
                try
                {
                    Debug.Assert(_exception == null && _requests.Count > 0);
                    _exception = ex.get();
                    _reference.getInstance().clientThreadPool().dispatch(delegate()
                                                                        {
                                                                            flushRequestsWithException(ex);
                                                                        });
                }
                finally
                {
                    _m.Unlock();
                }
            }
            catch(Ice.LocalException ex)
            {
                _m.Lock();
                try
                {
                    Debug.Assert(_exception == null && _requests.Count > 0);
                    _exception = ex;
                    _reference.getInstance().clientThreadPool().dispatch(delegate()
                                                                        {
                                                                            flushRequestsWithException(ex);
                                                                        });
                }
                finally
                {
                    _m.Unlock();
                }
            }

            if(sentCallbacks.Count > 0)
            {
                Instance instance = _reference.getInstance();
                instance.clientThreadPool().dispatch(delegate()
                                                    {
                                                        foreach(Request r in sentCallbacks)
                                                        {
                                                            if(r.@out != null)
                                                            {
                                                                r.@out.sent__(r.sentCallback);
                                                            }
                                                            else if(r.batchOut != null)
                                                            {
                                                                r.batchOut.sent__(r.sentCallback);
                                                            }
                                                        }
                                                    });
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
                _proxy.setRequestHandler__(_delegate, new ConnectionRequestHandler(_reference, _connection, _compress));
            }

            _m.Lock();
            try
            {
                Debug.Assert(!_initialized);
                if(_exception == null)
                {
                    _initialized = true;
                    _flushing = false;
                }
                _proxy = null; // Break cyclic reference count.
                _delegate = null; // Break cyclic reference count.
                _m.NotifyAll();
            }
            finally
            {
                _m.Unlock();
            }
        }

        private void
        flushRequestsWithException(Ice.LocalException ex)
        {
            foreach(Request request in _requests)
            {
                if(request.@out != null)
                {
                    request.@out.finished__(ex, false);
                }
                else if(request.batchOut != null)
                {
                    request.batchOut.finished__(ex, false);
                }
            }
            _requests.Clear();
        }

        private void
        flushRequestsWithException(LocalExceptionWrapper ex)
        {
            foreach(Request request in _requests)
            {
                if(request.@out != null)
                {
                    request.@out.finished__(ex);
                }
                else if(request.batchOut != null)
                {
                    request.batchOut.finished__(ex.get(), false);
                }
            }
            _requests.Clear();
        }

        private Reference _reference;
        private bool _response;

        private Ice.ObjectPrxHelperBase _proxy;
        private Ice.ObjectDelM_ _delegate;

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

        private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
    }
}
