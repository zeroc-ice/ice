// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
            os.rewriteInt(value, pos);
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
            _requestId = 0;
        }

        public RequestHandler update(RequestHandler previousHandler, RequestHandler newHandler)
        {
            return previousHandler == this ? newHandler : this;
        }

        public bool sendAsyncRequest(ProxyOutgoingAsyncBase outAsync, out Ice.AsyncCallback sentCallback)
        {
            return outAsync.invokeCollocated(this, out sentCallback);
        }

        public void asyncRequestCanceled(OutgoingAsyncBase outAsync, Ice.LocalException ex)
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
                    Ice.AsyncCallback cb = outAsync.completed(ex);
                    if(cb != null)
                    {
                        outAsync.invokeCompletedAsync(cb);
                    }
                    _adapter.decDirectCount(); // invokeAll won't be called, decrease the direct count.
                    return;
                }
                if(outAsync is OutgoingAsync)
                {
                    OutgoingAsync o = (OutgoingAsync)outAsync;
                    Debug.Assert(o != null);
                    foreach(KeyValuePair<int, OutgoingAsyncBase> e in _asyncRequests)
                    {
                        if(e.Value == o)
                        {
                            _asyncRequests.Remove(e.Key);
                            Ice.AsyncCallback cb = outAsync.completed(ex);
                            if(cb != null)
                            {
                                outAsync.invokeCompletedAsync(cb);
                            }
                            return;
                        }
                    }
                }
            }
        }

        public void sendResponse(int requestId, BasicStream os, byte status, bool amd)
        {
            Ice.AsyncCallback cb = null;
            OutgoingAsyncBase outAsync;
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
                    _asyncRequests.Remove(requestId);
                    outAsync.getIs().swap(os);
                    cb = outAsync.completed();
                }
            }

            if(cb != null)
            {
                if(amd)
                {
                    outAsync.invokeCompletedAsync(cb);
                }
                else
                {
                    outAsync.invokeCompleted(cb);
                }
            }
            _adapter.decDirectCount();
        }

        public void
        sendNoResponse()
        {
            _adapter.decDirectCount();
        }

        public bool
        systemException(int requestId, Ice.SystemException ex, bool amd)
        {
            handleException(requestId, ex, amd);
            _adapter.decDirectCount();
            return true;
        }

        public void
        invokeException(int requestId, Ice.LocalException ex, int invokeNum, bool amd)
        {
            handleException(requestId, ex, amd);
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

        public bool invokeAsyncRequest(OutgoingAsyncBase outAsync, int batchRequestNum, bool synchronous,
                                       out Ice.AsyncCallback sentCallback)
        {
            //
            // Increase the direct count to prevent the thread pool from being destroyed before
            // invokeAll is called. This will also throw if the object adapter has been deactivated.
            //
            _adapter.incDirectCount();

            int requestId = 0;
            try
            {
                lock(this)
                {
                    outAsync.cancelable(this); // This will throw if the request is canceled

                    if(_response)
                    {
                        requestId = ++_requestId;
                        _asyncRequests.Add(requestId, outAsync);
                    }

                    _sendAsyncRequests.Add(outAsync, requestId);
                }
            }
            catch(System.Exception ex)
            {
                _adapter.decDirectCount();
                throw ex;
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
                            invokeAll(outAsync.getOs(), requestId, batchRequestNum);
                        }
                    }, null);
                }
                else if(_dispatcher)
                {
                    _adapter.getThreadPool().dispatchFromThisThread(() =>
                    {
                        if(sentAsync(outAsync))
                        {
                            invokeAll(outAsync.getOs(), requestId, batchRequestNum);
                        }
                    }, null);
                }
                else // Optimization: directly call invokeAll if there's no dispatcher.
                {
                    if(sentAsync(outAsync))
                    {
                        invokeAll(outAsync.getOs(), requestId, batchRequestNum);
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
                        invokeAll(outAsync.getOs(), requestId, batchRequestNum);
                    }
                }, null);
                sentCallback = null;
            }
            return false;
        }

        private bool sentAsync(OutgoingAsyncBase outAsync)
        {
            lock(this)
            {
                if(!_sendAsyncRequests.Remove(outAsync))
                {
                    return false; // The request timed-out.
                }
            }

            Ice.AsyncCallback cb = outAsync.sent();
            if(cb != null)
            {
                outAsync.invokeSent(cb);
            }
            return true;
        }

        private void invokeAll(BasicStream os, int requestId, int batchRequestNum)
        {
            if(batchRequestNum > 0)
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
                else if(batchRequestNum > 0)
                {
                    fillInValue(os, Protocol.headerSize, batchRequestNum);
                }
                TraceUtil.traceSend(os, _logger, _traceLevels);
            }

            int invokeNum = batchRequestNum > 0 ? batchRequestNum : 1;
            ServantManager servantManager = _adapter.getServantManager();
            try
            {
                while(invokeNum > 0)
                {
                    //
                    // Increase the direct count for the dispatch. We increase it again here for
                    // each dispatch. It's important for the direct count to be > 0 until the last
                    // collocated request response is sent to make sure the thread pool isn't
                    // destroyed before.
                    //
                    try
                    {
                        _adapter.incDirectCount();
                    }
                    catch(Ice.ObjectAdapterDeactivatedException ex)
                    {
                        handleException(requestId, ex, false);
                        break;
                    }

                    Incoming @in = new Incoming(_reference.getInstance(), this, null, _adapter, _response, (byte)0,
                                               requestId);
                    @in.invoke(servantManager, os);
                    --invokeNum;
                }
            }
            catch(Ice.LocalException ex)
            {
                invokeException(requestId, ex, invokeNum, false); // Fatal invocation exception
            }

            _adapter.decDirectCount();
        }

        void
        handleException(int requestId, Ice.Exception ex, bool amd)
        {
            if(requestId == 0)
            {
                return; // Ignore exception for oneway messages.
            }

            OutgoingAsyncBase outAsync;
            Ice.AsyncCallback cb = null;
            lock(this)
            {
                if(_asyncRequests.TryGetValue(requestId, out outAsync))
                {
                    _asyncRequests.Remove(requestId);
                    cb = outAsync.completed(ex);
                }
            }

            if(cb != null)
            {
                if(amd)
                {
                    outAsync.invokeCompletedAsync(cb);
                }
                else
                {
                    outAsync.invokeCompleted(cb);
                }
            }
        }

        private readonly Reference _reference;
        private readonly bool _dispatcher;
        private readonly bool _response;
        private readonly Ice.ObjectAdapterI _adapter;
        private readonly Ice.Logger _logger;
        private readonly TraceLevels _traceLevels;

        private int _requestId;

        private Dictionary<OutgoingAsyncBase, int> _sendAsyncRequests = new Dictionary<OutgoingAsyncBase, int>();
        private Dictionary<int, OutgoingAsyncBase> _asyncRequests = new Dictionary<int, OutgoingAsyncBase>();
    }
}
