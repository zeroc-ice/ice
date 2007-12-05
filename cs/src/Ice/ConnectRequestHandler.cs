// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

namespace IceInternal
{
    public class ConnectRequestHandler : RequestHandler, Reference.GetConnectionCallback, RouterInfo.AddProxyCallback
    {
        private class Request
        {
            internal Request(BasicStream os)
            {
                this.os = new BasicStream(os.instance());
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
        }

        public void prepareBatchRequest(BasicStream os)
        {
            lock(this)
            {
                while(_batchRequestInProgress)
                {
                    Monitor.Wait(this);
                }

                if(!initialized())
                {
                    _batchStream.swap(os);
                    _batchRequestInProgress = true;
                    return;
                }
            }

            _connection.prepareBatchRequest(os);
        }

        public void finishBatchRequest(BasicStream os)
        {
            lock(this)
            {
                if(!initialized())
                {
                    Debug.Assert(_batchRequestInProgress);
                    _batchRequestInProgress = false;
                    Monitor.PulseAll(this);

                    _batchStream.swap(os);

                    if(!_batchAutoFlush &&
                       _batchStream.size() + _batchRequestsSize > _reference.getInstance().messageSizeMax())
                    {
                        throw new Ice.MemoryLimitException();
                    }
                    _requests.Add(new Request(_batchStream));
                    return;
                }
            }

            _connection.finishBatchRequest(os, _compress);
        }

        public void abortBatchRequest()
        {
            lock(this)
            {
                if(!initialized())
                {
                    Debug.Assert(_batchRequestInProgress);
                    _batchRequestInProgress = false;
                    Monitor.PulseAll(this);

                    BasicStream dummy = new BasicStream(_reference.getInstance(), _batchAutoFlush);
                    _batchStream.swap(dummy);
                    _batchRequestsSize = Protocol.requestBatchHdr.Length;
                    return;
                }
            }

            _connection.abortBatchRequest();
        }

        public Ice.ConnectionI sendRequest(Outgoing @out)
        {
            return (!getConnection(true).sendRequest(@out, _compress, _response) || _response) ? _connection : null;
        }

        public void sendAsyncRequest(OutgoingAsync @out)
        {
            try
            {
                lock(this)
                {
                    if(!initialized())
                    {
                        _requests.Add(new Request(@out));
                        return;
                    }
                }

                _connection.sendAsyncRequest(@out, _compress, _response);
            }
            catch(LocalExceptionWrapper ex)
            {
                @out.finished__(ex);
            }
            catch(Ice.LocalException ex)
            {
                @out.finished__(ex);
            }
        }

        public bool flushBatchRequests(BatchOutgoing @out)
        {
            return getConnection(true).flushBatchRequests(@out);
        }

        public void flushAsyncBatchRequests(BatchOutgoingAsync @out)
        {
            try
            {
                lock(this)
                {
                    if(!initialized())
                    {
                        _requests.Add(new Request(@out));
                        return;
                    }
                }

                _connection.flushAsyncBatchRequests(@out);
            }
            catch(Ice.LocalException ex)
            {
                @out.finished__(ex);
            }
        }

        public Outgoing getOutgoing(string operation, Ice.OperationMode mode, Dictionary<string, string> context)
        {
            lock(this)
            {
                if(!initialized())
                {
                    return new IceInternal.Outgoing(this, operation, mode, context);
                }
            }

            return _connection.getOutgoing(this, operation, mode, context);
        }

        public void reclaimOutgoing(Outgoing og)
        {
            lock(this)
            {
                if(_connection == null)
                {
                    return;
                }
            }

            _connection.reclaimOutgoing(og);
        }

        public Reference getReference()
        {
            return _reference;
        }

        public Ice.ConnectionI getConnection(bool wait)
        {
            lock(this)
            {
                if(wait)
                {
                    //
                    // Wait for the connection establishment to complete or fail.
                    //
                    while(!_initialized && _exception == null)
                    {
                        Monitor.Wait(this);
                    }
                }

                if(_exception != null)
                {
                    throw _exception;
                }
                else
                {
                    Debug.Assert(!wait || _initialized);
                    return _connection;
                }
            }
        }

        //
        // Implementation of Reference.GetConnectionCallback
        //

        public void setConnection(Ice.ConnectionI connection, bool compress)
        {
            lock(this)
            {
                Debug.Assert(_connection == null && _exception == null);
                _connection = connection;
                _compress = compress;
            }

            //
            // If this proxy is for a non-local object, and we are using a router, then
            // add this proxy to the router info object.
            //
            RouterInfo ri = _reference.getRouterInfo();
            if(ri != null)
            {
                if(!ri.addProxy(_proxy, this))
                {
                    return; // The request handler will be initialized once addProxy returns.
                }
            }

            flushRequests();
        }

        public void setException(Ice.LocalException ex)
        {
            lock(this)
            {
                Debug.Assert(!_initialized && _exception == null);
                _exception = ex;
                _proxy = null; // Break cyclic reference count.
                _delegate = null; // Break cyclic reference count.
                Monitor.PulseAll(this);
            }

            foreach(Request request in _requests)
            {
                if(request.@out != null)
                {
                    request.@out.finished__(ex);
                }
                else if(request.batchOut != null)
                {
                    request.batchOut.finished__(ex);
                }
            }
            _requests.Clear();
        }

        //
        // Implementation of RouterInfo.AddProxyCallback
        //
        public void addedProxy()
        {
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
            _batchStream = new BasicStream(@ref.getInstance(), _batchAutoFlush);
            _batchRequestInProgress = false;
            _batchRequestsSize = Protocol.requestBatchHdr.Length;
            _updateRequestHandler = false;
        }

        public RequestHandler connect()
        {
            _reference.getConnection(this);

            lock(this)
            {
                if(_connection != null)
                {
                    return new ConnectionRequestHandler(_reference, _connection, _compress);
                }
                else
                {
                    _updateRequestHandler = true;
                    return this;
                }
            }
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
                    Monitor.Wait(this);
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

                if(_batchRequestInProgress)
                {
                    Monitor.Wait(this);
                }

                //
                // We set the _flushing flag to true to prevent any additional queuing. Callers
                // might block for a little while as the queued requests are being sent but this
                // shouldn't be an issue as the request sends are non-blocking.
                //
                _flushing = true;
            }

            foreach(Request request in _requests) // _requests is immutable when _flushing = true
            {
                if(request.@out != null)
                {
                    try
                    {
                        _connection.sendAsyncRequest(request.@out, _compress, _response);
                    }
                    catch(LocalExceptionWrapper ex)
                    {
                        request.@out.finished__(ex);
                    }
                    catch(Ice.LocalException ex)
                    {
                        request.@out.finished__(ex);
                    }
                }
                else if(request.batchOut != null)
                {
                    try
                    {
                        _connection.flushAsyncBatchRequests(request.batchOut);
                    }
                    catch(Ice.LocalException ex)
                    {
                        request.batchOut.finished__(ex);
                    }
                }
                else
                {
                    //
                    // TODO: Add sendBatchRequest() method to ConnectionI?
                    //
                    try
                    {
                        BasicStream os = new BasicStream(request.os.instance());
                        _connection.prepareBatchRequest(os);
                        request.os.pos(0);
                        os.writeBlob(request.os.readBlob(request.os.size()));
                        _connection.finishBatchRequest(os, _compress);
                    }
                    catch(Ice.LocalException)
                    {
                        _connection.abortBatchRequest();
                        throw;
                    }
                }
            }
            _requests.Clear();

            lock(this)
            {
                _initialized = true;
                _flushing = false;
                Monitor.PulseAll(this);
            }

            if(_updateRequestHandler && _exception == null)
            {
                _proxy.setRequestHandler__(_delegate, new ConnectionRequestHandler(_reference, _connection, _compress));
            }
            _proxy = null; // Break cyclic reference count.
            _delegate = null; // Break cyclic reference count.
        }

        private Reference _reference;
        private bool _batchAutoFlush;
        private Ice.ObjectPrxHelperBase _proxy;
        private Ice.ObjectDelM_ _delegate;
        private bool _initialized = false;
        private bool _flushing = false;
        private Ice.ConnectionI _connection = null;
        private bool _compress = false;
        private bool _response;
        private Ice.LocalException _exception = null;

        private List<Request> _requests = new List<Request>();
        private bool _batchRequestInProgress;
        private int _batchRequestsSize;
        private BasicStream _batchStream;
        private bool _updateRequestHandler;
    }
}
