// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
        fillInValue(Ice.OutputStream os, int pos, int value)
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

        public int sendAsyncRequest(ProxyOutgoingAsyncBase outAsync)
        {
            return outAsync.invokeCollocated(this);
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
                    if(outAsync.exception(ex))
                    {
                        outAsync.invokeExceptionAsync();
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
                            if(outAsync.exception(ex))
                            {
                                outAsync.invokeExceptionAsync();
                            }
                            return;
                        }
                    }
                }
            }
        }

        public void sendResponse(int requestId, Ice.OutputStream os, byte status, bool amd)
        {
            OutgoingAsyncBase outAsync;
            lock(this)
            {
                Debug.Assert(_response);

                if(_traceLevels.protocol >= 1)
                {
                    fillInValue(os, 10, os.size());
                }

                // Adopt the OutputStream's buffer.
                Ice.InputStream iss = new Ice.InputStream(os.instance(), os.getEncoding(), os.getBuffer(), true);

                iss.pos(Protocol.replyHdr.Length + 4);

                if(_traceLevels.protocol >= 1)
                {
                    TraceUtil.traceRecv(iss, _logger, _traceLevels);
                }

                if(_asyncRequests.TryGetValue(requestId, out outAsync))
                {
                    outAsync.getIs().swap(iss);
                    if(!outAsync.response())
                    {
                        outAsync = null;
                    }
                    _asyncRequests.Remove(requestId);
                }
            }

            if(outAsync != null)
            {
                if(amd)
                {
                    outAsync.invokeResponseAsync();
                }
                else
                {
                    outAsync.invokeResponse();
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

        public int invokeAsyncRequest(OutgoingAsyncBase outAsync, int batchRequestNum, bool synchronous)
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
            catch(Exception)
            {
                _adapter.decDirectCount();
                throw;
            }

            outAsync.attachCollocatedObserver(_adapter, requestId);
            if(!synchronous || !_response || _reference.getInvocationTimeout() > 0)
            {
                // Don't invoke from the user thread if async or invocation timeout is set
                _adapter.getThreadPool().dispatch(
                    () =>
                    {
                        if (sentAsync(outAsync))
                        {
                            invokeAll(outAsync.getOs(), requestId, batchRequestNum);
                        }
                    }, null);
            }
            else if(_dispatcher)
            {
                _adapter.getThreadPool().dispatchFromThisThread(
                    () =>
                    {
                        if (sentAsync(outAsync))
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
            return OutgoingAsyncBase.AsyncStatusQueued;
        }

        private bool sentAsync(OutgoingAsyncBase outAsync)
        {
            lock(this)
            {
                if(!_sendAsyncRequests.Remove(outAsync))
                {
                    return false; // The request timed-out.
                }

                if(!outAsync.sent())
                {
                    return true;
                }
            }
            outAsync.invokeSent();
            return true;
        }

        private void invokeAll(Ice.OutputStream os, int requestId, int batchRequestNum)
        {
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

            Ice.InputStream iss = new Ice.InputStream(os.instance(), os.getEncoding(), os.getBuffer(), false);

            if(batchRequestNum > 0)
            {
                iss.pos(Protocol.requestBatchHdr.Length);
            }
            else
            {
                iss.pos(Protocol.requestHdr.Length);
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

                    Incoming inS = new Incoming(_reference.getInstance(), this, null, _adapter, _response, (byte)0,
                                                requestId);
                    inS.invoke(servantManager, iss);
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
            lock(this)
            {
                if(_asyncRequests.TryGetValue(requestId, out outAsync))
                {
                    if(!outAsync.exception(ex))
                    {
                        outAsync = null;
                    }
                    _asyncRequests.Remove(requestId);
                }
            }

            if(outAsync != null)
            {
                //
                // If called from an AMD dispatch, invoke asynchronously
                // the completion callback since this might be called from
                // the user code.
                //
                if(amd)
                {
                    outAsync.invokeExceptionAsync();
                }
                else
                {
                    outAsync.invokeException();
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
