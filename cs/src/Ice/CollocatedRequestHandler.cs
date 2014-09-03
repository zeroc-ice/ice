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
    public class CollocatedRequestHandler : RequestHandler, ResponseHandler
    {
        private void
        fillInValue(BasicStream os, int pos, int value)
        {
            os.rewriteInt(pos, value);
        }

        public
        CollocatedRequestHandler(Reference @ref, Ice.ObjectAdapter adapter)
        {
            _reference = @ref;
            _dispatcher = _reference.getInstance().initializationData().dispatcher != null;
            _response = _reference.getMode() == Reference.Mode.ModeTwoway;
            _adapter = (Ice.ObjectAdapterI)adapter;

            _logger = _reference.getInstance().initializationData().logger; // Cached for better performance.
            _traceLevels = _reference.getInstance().traceLevels(); // Cached for better performance.
            _batchAutoFlush = @ref.getInstance().initializationData().properties.getPropertyAsIntWithDefault(
                "Ice.BatchAutoFlush", 1) > 0;
            _requestId = 0;
            _batchStreamInUse = false;
            _batchRequestNum = 0;
            _batchStream = new BasicStream(@ref.getInstance(), Ice.Util.currentProtocolEncoding, _batchAutoFlush);
        }

        public void
        prepareBatchRequest(BasicStream os)
        {
            lock(this)
            {
                while(_batchStreamInUse)
                {
                    Monitor.Wait(this);
                }

                if(_batchStream.isEmpty())
                {
                    try
                    {
                        _batchStream.writeBlob(Protocol.requestBatchHdr);
                    }
                    catch(Ice.LocalException ex)
                    {
                        throw ex;
                    }
                }

                _batchStreamInUse = true;
                _batchMarker = _batchStream.size();
                _batchStream.swap(os);
            }
        }

        public void finishBatchRequest(BasicStream os)
        {
            try
            {
                lock(this)
                {
                    _batchStream.swap(os);

                    if(_batchAutoFlush & (_batchStream.size() > _reference.getInstance().messageSizeMax()))
                    {
                        //
                        // Temporarily save the last request.
                        //
                        byte[] lastRequest = new byte[_batchStream.size() - _batchMarker];
                        Buffer buffer = _batchStream.getBuffer();
                        buffer.b.position(_batchMarker);
                        buffer.b.get(lastRequest);
                        _batchStream.resize(_batchMarker, false);

                        int invokeNum = _batchRequestNum;
                        BasicStream stream = new BasicStream(_reference.getInstance(),
                                                             Ice.Util.currentProtocolEncoding,
                                                             _batchAutoFlush);
                        stream.swap(_batchStream);

                        _adapter.getThreadPool().dispatch(() =>
                        {
                            invokeAll(stream, 0, invokeNum, true);
                        }, null);

                        //
                        // Reset the batch.
                        //
                        _batchRequestNum = 0;
                        _batchMarker = 0;

                        //
                        // Check again if the last request doesn't exceed what we can send with the auto flush
                        //
                        if(Protocol.requestBatchHdr.Length + lastRequest.Length >
                           _reference.getInstance().messageSizeMax())
                        {
                            Ex.throwMemoryLimitException(Protocol.requestBatchHdr.Length + lastRequest.Length,
                                                         _reference.getInstance().messageSizeMax());
                        }

                        //
                        // Start a new batch with the last message that caused us to go over the limit.
                        //
                        _batchStream.writeBlob(Protocol.requestBatchHdr);
                        _batchStream.writeBlob(lastRequest);
                    }

                    //
                    // Increment the number of requests in the batch.
                    //
                    Debug.Assert(_batchStreamInUse);
                    ++_batchRequestNum;
                    _batchStreamInUse = false;
                    Monitor.PulseAll(this);
                }
            }
            catch(Ice.LocalException ex)
            {
                abortBatchRequest();
                throw ex;
            }
        }

        public void abortBatchRequest()
        {
            lock(this)
            {
                BasicStream dummy = new BasicStream(_reference.getInstance(), Ice.Util.currentProtocolEncoding,
                                                    _batchAutoFlush);
                _batchStream.swap(dummy);
                _batchRequestNum = 0;
                _batchMarker = 0;

                Debug.Assert(_batchStreamInUse);
                _batchStreamInUse = false;
                Monitor.PulseAll(this);
            }
        }

        public bool sendAsyncRequest(OutgoingAsyncMessageCallback outAsync, out Ice.AsyncCallback sentCallback)
        {
            return outAsync.invokeCollocated(this, out sentCallback);
        }

        public void asyncRequestCanceled(OutgoingAsyncMessageCallback outAsync, Ice.LocalException ex)
        {
            lock(this)
            {
                int requestId;
                if(_sendAsyncRequests.TryGetValue(outAsync, out requestId))
                {
                    if(requestId > 0)
                    {
                        _asyncRequests.Remove(requestId);
                    }
                    _sendAsyncRequests.Remove(outAsync);
                    outAsync.dispatchInvocationCancel(ex, _reference.getInstance().clientThreadPool(), null);
                    return; // We're done
                }
                if(outAsync is OutgoingAsync)
                {
                    OutgoingAsync o = (OutgoingAsync)outAsync;
                    Debug.Assert(o != null);
                    foreach(KeyValuePair<int, OutgoingAsync> e in _asyncRequests)
                    {
                        if(e.Value == o)
                        {
                            _asyncRequests.Remove(e.Key);
                            outAsync.dispatchInvocationCancel(ex, _reference.getInstance().clientThreadPool(), null);
                            return; // We're done
                        }
                    }
                }
            }
        }

        public void sendResponse(int requestId, BasicStream os, byte status)
        {
            OutgoingAsync outAsync = null;
            lock(this)
            {
                Debug.Assert(_response);

                os.pos(Protocol.replyHdr.Length + 4);

                if(_traceLevels.protocol >= 1)
                {
                    fillInValue(os, 10, os.size());
                    TraceUtil.traceRecv(os, _logger, _traceLevels);
                }

                if(_asyncRequests.TryGetValue(requestId, out outAsync))
                {
                    os.swap(outAsync.istr__);
                    _asyncRequests.Remove(requestId);
                }
            }

            if(outAsync != null)
            {
                outAsync.finished();
            }
            _adapter.decDirectCount();
        }

        public void
        sendNoResponse()
        {
            _adapter.decDirectCount();
        }

        public bool
        systemException(int requestId, Ice.SystemException ex)
        {
            handleException(requestId, ex);
            _adapter.decDirectCount();
            return true;
        }
        
        public void
        invokeException(int requestId, Ice.LocalException ex, int invokeNum)
        {
            if(requestId > 0)
            {
                OutgoingAsync outAsync = null;
                lock(this)
                {
                    if(_asyncRequests.TryGetValue(requestId, out outAsync))
                    {
                        _asyncRequests.Remove(requestId);
                    }
                }
                if(outAsync != null)
                {
                    outAsync.finished(ex);
                }
            }
            _adapter.decDirectCount();
        }

        public Reference
        getReference()
        {
            return _reference;
        }

        public Ice.ConnectionI
        getConnection()
        {
            return null;
        }

        public Ice.ConnectionI
        waitForConnection()
        {
            return null;
        }

        public void invokeAsyncRequest(OutgoingAsync outAsync, bool synchronous, out Ice.AsyncCallback sentCallback)
        {
            int requestId = 0;
            if(_reference.getInvocationTimeout() > 0 || _response)
            {
                lock(this)
                {
                    if(_response)
                    {
                        requestId = ++_requestId;
                        _asyncRequests.Add(requestId, outAsync);
                    }
                    if(_reference.getInvocationTimeout() > 0)
                    {
                        _sendAsyncRequests.Add(outAsync, requestId);
                    }
                }
            }

            outAsync.attachCollocatedObserver(_adapter, requestId);

            if(synchronous)
            {
                //
                // Treat this collocated call as if it is a synchronous invocation.
                //
                if(_reference.getInvocationTimeout() > 0 || !_response)
                {
                    // Don't invoke from the user thread, invocation timeouts wouldn't work otherwise.
                    _adapter.getThreadPool().dispatch(() =>
                    {
                        if(sentAsync(outAsync))
                        {
                            invokeAll(outAsync.ostr__, requestId, 1, false);
                        }
                    }, null);
                }
                else if(_dispatcher)
                {
                    _adapter.getThreadPool().dispatchFromThisThread(() =>
                    {
                        if(sentAsync(outAsync))
                        {
                            invokeAll(outAsync.ostr__, requestId, 1, false);
                        }
                    }, null);
                }
                else // Optimization: directly call invokeAll if there's no dispatcher.
                {
                    if(sentAsync(outAsync))
                    {
                        invokeAll(outAsync.ostr__, requestId, 1, false);
                    }
                }
                sentCallback = null;
            }
            else
            {
                _adapter.getThreadPool().dispatch(() =>
                {
                    if(sentAsync(outAsync))
                    {
                        invokeAll(outAsync.ostr__, requestId, 1, false);
                    }
                }, null);
                sentCallback = null;
            }
        }

        public bool invokeAsyncBatchRequests(BatchOutgoingAsync outAsync, out Ice.AsyncCallback sentCallback)
        {
            int invokeNum;
            lock(this)
            {
                while(_batchStreamInUse)
                {
                    Monitor.Wait(this);
                }

                invokeNum = _batchRequestNum;
                if(_batchRequestNum > 0)
                {
                    if(_reference.getInvocationTimeout() > 0)
                    {
                        _sendAsyncRequests.Add(outAsync, 0);
                    }

                    Debug.Assert(!_batchStream.isEmpty());
                    _batchStream.swap(outAsync.ostr__);

                    //
                    // Reset the batch stream.
                    //
                    BasicStream dummy = new BasicStream(_reference.getInstance(), Ice.Util.currentProtocolEncoding,
                                                        _batchAutoFlush);
                    _batchStream.swap(dummy);
                    _batchRequestNum = 0;
                    _batchMarker = 0;
                }
            }

            outAsync.attachCollocatedObserver(_adapter, 0);

            if(invokeNum > 0)
            {
                _adapter.getThreadPool().dispatch(() =>
                {
                    if(sentAsync(outAsync))
                    {
                        invokeAll(outAsync.ostr__, 0, invokeNum, true);
                    }
                }, null);
                sentCallback = null;
                return false;
            }
            else
            {
                sentCallback = outAsync.sent();
                return true;
            }
        }


        private bool sentAsync(OutgoingAsyncMessageCallback outAsync)
        {
            if(_reference.getInvocationTimeout() > 0)
            {
                lock(this)
                {
                    if(!_sendAsyncRequests.Remove(outAsync))
                    {
                        return false; // The request timed-out.
                    }
                }
            }

            Ice.AsyncCallback cb = outAsync.sent();
            if(cb != null)
            {
                outAsync.invokeSent(cb);
            }
            return true;
        }

        private void invokeAll(BasicStream os, int requestId, int invokeNum, bool batch)
        {
            if(batch)
            {
                os.pos(Protocol.requestBatchHdr.Length);
            }
            else
            {
                os.pos(Protocol.requestHdr.Length);
            }

            if(_traceLevels.protocol >= 1)
            {
                fillInValue(os, 10, os.size());
                if(requestId > 0)
                {
                    fillInValue(os, Protocol.headerSize, requestId);
                }
                else if(batch)
                {
                    fillInValue(os, Protocol.headerSize, invokeNum);
                }
                TraceUtil.traceSend(os, _logger, _traceLevels);
            }

            ServantManager servantManager = _adapter.getServantManager();
            try
            {
                while(invokeNum > 0)
                {
                    try
                    {
                        _adapter.incDirectCount();
                    }
                    catch(Ice.ObjectAdapterDeactivatedException ex)
                    {
                        handleException(requestId, ex);
                        return;
                    }

                    Incoming @in = new Incoming(_reference.getInstance(), this, null, _adapter, _response, (byte)0,
                                               requestId);
                    @in.invoke(servantManager, os);
                    --invokeNum;
                }
            }
            catch(Ice.LocalException ex)
            {
                invokeException(requestId, ex, invokeNum); // Fatal invocation exception
            }
        }

        void
        handleException(int requestId, Ice.Exception ex)
        {
            if(requestId == 0)
            {
                return; // Ignore exception for oneway messages.
            }

            OutgoingAsync outAsync = null;
            lock(this)
            {
                if(_asyncRequests.TryGetValue(requestId, out outAsync))
                {
                    _asyncRequests.Remove(requestId);
                }
            }

            if(outAsync != null)
            {
                outAsync.finished(ex);
            }
        }

        private readonly Reference _reference;
        private readonly bool _dispatcher;
        private readonly bool _response;
        private readonly Ice.ObjectAdapterI _adapter;
        private readonly Ice.Logger _logger;
        private readonly TraceLevels _traceLevels;
        private bool _batchAutoFlush;

        private int _requestId;

        private Dictionary<OutgoingAsyncMessageCallback, int> _sendAsyncRequests =
            new Dictionary<OutgoingAsyncMessageCallback, int>();
        private Dictionary<int, OutgoingAsync> _asyncRequests = new Dictionary<int, OutgoingAsync>();

        private BasicStream _batchStream;
        private bool _batchStreamInUse;
        private int _batchRequestNum;
        private int _batchMarker;
    }
}