// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class CollocatedRequestHandler implements RequestHandler, ResponseHandler
{
    private class InvokeAllAsync extends DispatchWorkItem
    {
        private InvokeAllAsync(OutgoingAsyncBase outAsync, BasicStream os, int requestId, int invokeNum, boolean batch)
        {
            _outAsync = outAsync;
            _os = os;
            _requestId = requestId;
            _invokeNum = invokeNum;
            _batch = batch;
        }

        @Override
        public void run()
        {
            if(sentAsync(_outAsync))
            {
                invokeAll(_os, _requestId, _invokeNum, _batch);
            }
        }

        private final OutgoingAsyncBase _outAsync;
        private BasicStream _os;
        private final int _requestId;
        private final int _invokeNum;
        private final boolean _batch;
    };

    public
    CollocatedRequestHandler(Reference ref, Ice.ObjectAdapter adapter)
    {
        _reference = ref;
        _dispatcher = ref.getInstance().initializationData().dispatcher != null;
        _response = _reference.getMode() == Reference.ModeTwoway;
        _adapter = (Ice.ObjectAdapterI)adapter;

        _logger = _reference.getInstance().initializationData().logger; // Cached for better performance.
        _traceLevels = _reference.getInstance().traceLevels(); // Cached for better performance.
        _batchAutoFlushSize = ref.getInstance().batchAutoFlushSize();
        _requestId = 0;
        _batchStreamInUse = false;
        _batchRequestNum = 0;
        _batchStream = new BasicStream(ref.getInstance(), Protocol.currentProtocolEncoding);
    }

    @Override
    public RequestHandler 
    connect(Ice.ObjectPrxHelperBase proxy)
    {
        return this;
    }
    
    @Override
    public RequestHandler 
    update(RequestHandler previousHandler, RequestHandler newHandler)
    {
        return previousHandler == this ? newHandler : this;
    }
    
    @Override
    synchronized public void
    prepareBatchRequest(BasicStream os)
    {
        waitStreamInUse();
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

    @Override
    public void
    finishBatchRequest(BasicStream os)
    {
        try
        {
            synchronized(this)
            {
                _batchStream.swap(os);

                if(_batchAutoFlushSize > 0 && (_batchStream.size() > _batchAutoFlushSize))
                {
                    //
                    // Temporarily save the last request.
                    //
                    byte[] lastRequest = new byte[_batchStream.size() - _batchMarker];
                    Buffer buffer = _batchStream.getBuffer();
                    buffer.b.position(_batchMarker);
                    buffer.b.get(lastRequest);
                    _batchStream.resize(_batchMarker, false);

                    final int invokeNum = _batchRequestNum;
                    final BasicStream stream = new BasicStream(_reference.getInstance(),
                                                               Protocol.currentProtocolEncoding);
                    stream.swap(_batchStream);

                    _adapter.getThreadPool().dispatch(
                        new DispatchWorkItem()
                        {
                            @Override
                            public void
                            run()
                            {
                                CollocatedRequestHandler.this.invokeAll(stream, 0, invokeNum, true);
                            }
                        });

                    //
                    // Reset the batch.
                    //
                    _batchRequestNum = 0;
                    _batchMarker = 0;

                    //
                    // Start a new batch with the last message that caused us to go over the limit.
                    //
                    _batchStream.writeBlob(Protocol.requestBatchHdr);
                    _batchStream.writeBlob(lastRequest);
                }

                //
                // Increment the number of requests in the batch.
                //
                assert(_batchStreamInUse);
                ++_batchRequestNum;
                _batchStreamInUse = false;
                notifyAll();
            }
        }
        catch(Ice.LocalException ex)
        {
            abortBatchRequest();
            throw ex;
        }
    }

    @Override
    synchronized public void
    abortBatchRequest()
    {
        BasicStream dummy = new BasicStream(_reference.getInstance(), Protocol.currentProtocolEncoding);
        _batchStream.swap(dummy);
        _batchRequestNum = 0;
        _batchMarker = 0;

        assert(_batchStreamInUse);
        _batchStreamInUse = false;
        notifyAll();
    }

    @Override
    public int
    sendAsyncRequest(OutgoingAsyncBase outAsync)
    {
        return outAsync.invokeCollocated(this);
    }

    @Override
    synchronized public void
    asyncRequestCanceled(OutgoingAsyncBase outAsync, Ice.LocalException ex)
    {
        Integer requestId = _sendAsyncRequests.get(outAsync);
        if(requestId != null)
        {
            if(requestId > 0)
            {
                _asyncRequests.remove(requestId);
            }
            _sendAsyncRequests.remove(outAsync);
            if(outAsync.completed(ex))
            {
                outAsync.invokeCompletedAsync();
            }
            return;
        }

        if(outAsync instanceof OutgoingAsync)
        {
            OutgoingAsync o = (OutgoingAsync)outAsync;
            assert(o != null);
            for(java.util.Map.Entry<Integer, OutgoingAsync> e : _asyncRequests.entrySet())
            {
                if(e.getValue() == o)
                {
                    _asyncRequests.remove(e.getKey());
                    if(outAsync.completed(ex))
                    {
                        outAsync.invokeCompletedAsync();
                    }
                    return;
                }
            }
        }
    }

    @Override
    public void
    sendResponse(int requestId, final BasicStream os, byte status)
    {
        OutgoingAsync outAsync = null;
        synchronized(this)
        {
            assert(_response);

            os.pos(Protocol.replyHdr.length + 4);

            if(_traceLevels.protocol >= 1)
            {
                fillInValue(os, 10, os.size());
                TraceUtil.traceRecv(os, _logger, _traceLevels);
            }

            outAsync = _asyncRequests.remove(requestId);
            if(outAsync != null && !outAsync.completed(os))
            {
                outAsync = null;
            }
        }

        if(outAsync != null)
        {
            outAsync.invokeCompleted();
        }
        _adapter.decDirectCount();
    }

    @Override
    public void
    sendNoResponse()
    {
        _adapter.decDirectCount();
    }

    @Override
    public boolean
    systemException(int requestId, Ice.SystemException ex)
    {
        handleException(requestId, ex);
        _adapter.decDirectCount();
        return true;
    }

    @Override
    public void
    invokeException(int requestId, Ice.LocalException ex, int invokeNum)
    {
        handleException(requestId, ex);
        _adapter.decDirectCount();
    }

    @Override
    public Reference
    getReference()
    {
        return _reference;
    }

    @Override
    public Ice.ConnectionI
    getConnection()
    {
        return null;
    }

    @Override
    public Ice.ConnectionI
    waitForConnection()
    {
        return null;
    }

    int invokeAsyncRequest(OutgoingAsync outAsync, boolean synchronous)
    {
        int requestId = 0;
        if((_reference.getInstance().queueRequests() || _reference.getInvocationTimeout() > 0) || _response)
        {
            synchronized(this)
            {
                outAsync.cancelable(this); // This will throw if the request is canceled

                if(_response)
                {
                    requestId = ++_requestId;
                    _asyncRequests.put(requestId, outAsync);
                }
                if(_reference.getInstance().queueRequests() || _reference.getInvocationTimeout() > 0)
                {
                    _sendAsyncRequests.put(outAsync, requestId);
                }
            }
        }

        outAsync.attachCollocatedObserver(_adapter, requestId);

        if(synchronous)
        {
            //
            // Treat this collocated call as if it is a synchronous invocation.
            //
            if(_reference.getInstance().queueRequests() || _reference.getInvocationTimeout() > 0 || !_response)
            {
                // Don't invoke from the user thread, invocation timeouts wouldn't work otherwise.
                _adapter.getThreadPool().dispatch(new InvokeAllAsync(outAsync, outAsync.getOs(), requestId, 1, false));
            }
            else if(_dispatcher)
            {
                _adapter.getThreadPool().dispatchFromThisThread(
                        new InvokeAllAsync(outAsync, outAsync.getOs(), requestId, 1, false));
            }
            else // Optimization: directly call invokeAll if there's no dispatcher.
            {
                if(sentAsync(outAsync))
                {
                    invokeAll(outAsync.getOs(), requestId, 1, false);
                }
            }
        }
        else
        {
            _adapter.getThreadPool().dispatch(new InvokeAllAsync(outAsync, outAsync.getOs(), requestId, 1, false));
        }
        return AsyncStatus.Queued;
    }

    int invokeAsyncBatchRequests(OutgoingAsyncBase outAsync)
    {
        int invokeNum;
        synchronized(this)
        {
            waitStreamInUse();

            invokeNum = _batchRequestNum;
            if(_batchRequestNum > 0)
            {
                outAsync.cancelable(this); // This will throw if the request is canceled

                if(_reference.getInstance().queueRequests() || _reference.getInvocationTimeout() > 0)
                {
                    _sendAsyncRequests.put(outAsync, 0);
                }

                assert(!_batchStream.isEmpty());
                _batchStream.swap(outAsync.getOs());

                //
                // Reset the batch stream.
                //
                BasicStream dummy = new BasicStream(_reference.getInstance(), Protocol.currentProtocolEncoding);
                _batchStream.swap(dummy);
                _batchRequestNum = 0;
                _batchMarker = 0;
            }
        }

        outAsync.attachCollocatedObserver(_adapter, 0);

        if(invokeNum > 0)
        {
            _adapter.getThreadPool().dispatch(new InvokeAllAsync(outAsync, outAsync.getOs(), 0, invokeNum, true));
            return AsyncStatus.Queued;
        }
        else if(outAsync.sent())
        {
            return AsyncStatus.Sent | AsyncStatus.InvokeSentCallback;
        }
        else
        {
            return AsyncStatus.Sent;
        }
    }

    private boolean
    sentAsync(final OutgoingAsyncBase outAsync)
    {
        if(_reference.getInstance().queueRequests() || _reference.getInvocationTimeout() > 0)
        {
            synchronized(this)
            {
                if(_sendAsyncRequests.remove(outAsync) == null)
                {
                    return false; // The request timed-out.
                }
            }
        }

        if(outAsync.sent())
        {
            outAsync.invokeSent();
        }
        return true;
    }

    private void
    invokeAll(BasicStream os, int requestId, int invokeNum, boolean batch)
    {
        if(batch)
        {
            os.pos(Protocol.requestBatchHdr.length);
        }
        else
        {
            os.pos(Protocol.requestHdr.length);
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

                Incoming in = new Incoming(_reference.getInstance(), this, null, _adapter, _response, (byte)0,
                                           requestId);
                in.invoke(servantManager, os);
                --invokeNum;
            }
        }
        catch(Ice.LocalException ex)
        {
            invokeException(requestId, ex, invokeNum); // Fatal invocation exception
        }
        catch(java.lang.AssertionError ex) // Upon assertion, we print the stack trace.
        {
            Ice.UnknownException uex = new Ice.UnknownException(ex);
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            ex.printStackTrace(pw);
            pw.flush();
            uex.unknown = sw.toString();
            _logger.error(uex.unknown);
            invokeException(requestId, uex, invokeNum);
        }
        catch(java.lang.OutOfMemoryError ex)
        {
            Ice.UnknownException uex = new Ice.UnknownException(ex);
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            ex.printStackTrace(pw);
            pw.flush();
            uex.unknown = sw.toString();
            _logger.error(uex.unknown);
            invokeException(requestId, uex, invokeNum);
        }
    }

    private void
    handleException(int requestId, Ice.Exception ex)
    {
        if(requestId == 0)
        {
            return; // Ignore exception for oneway messages.
        }

        OutgoingAsync outAsync = null;
        synchronized(this)
        {
            outAsync = _asyncRequests.remove(requestId);
            if(outAsync != null && !outAsync.completed(ex))
            {
                outAsync = null;
            }
        }
        
        if(outAsync != null)
        {
            outAsync.invokeCompleted();
        }
    }

    private void
    waitStreamInUse()
    {
        //
        // This is similar to a mutex lock in that the stream is
        // only "locked" while marshaling. As such we don't permit the wait
        // to be interrupted. Instead the interrupted status is saved and
        // restored.
        //
        boolean interrupted = false;
        while(_batchStreamInUse)
        {
            try
            {
                wait();
            }
            catch(InterruptedException ex)
            {
                interrupted = true;
            }
        }
        //
        // Restore the interrupted flag if we were interrupted.
        //
        if(interrupted)
        {
            Thread.currentThread().interrupt();
        }
    }

    private void
    fillInValue(BasicStream os, int pos, int value)
    {
        os.rewriteInt(pos, value);
    }

    private final Reference _reference;
    private final boolean _dispatcher;
    private final boolean _response;
    private final Ice.ObjectAdapterI _adapter;
    private final Ice.Logger _logger;
    private final TraceLevels _traceLevels;
    private int _batchAutoFlushSize;

    private int _requestId;

    // A map of outstanding requests that can be canceled. A request
    // can be canceled if it has an invocation timeout, or we support
    // interrupts.
    private java.util.Map<OutgoingAsyncBase, Integer> _sendAsyncRequests =
        new java.util.HashMap<OutgoingAsyncBase, Integer>();

    private java.util.Map<Integer, OutgoingAsync> _asyncRequests = new java.util.HashMap<Integer, OutgoingAsync>();

    private BasicStream _batchStream;
    private boolean _batchStreamInUse;
    private int _batchRequestNum;
    private int _batchMarker;
}
