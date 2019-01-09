// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package IceInternal;

public class CollocatedRequestHandler implements RequestHandler, ResponseHandler
{
    private class InvokeAllAsync extends DispatchWorkItem
    {
        private InvokeAllAsync(OutgoingAsyncBase outAsync, Ice.OutputStream os, int requestId, int batchRequestNum)
        {
            _outAsync = outAsync;
            _os = os;
            _requestId = requestId;
            _batchRequestNum = batchRequestNum;
        }

        @Override
        public void run()
        {
            if(sentAsync(_outAsync))
            {
                invokeAll(_os, _requestId, _batchRequestNum);
            }
        }

        private final OutgoingAsyncBase _outAsync;
        private Ice.OutputStream _os;
        private final int _requestId;
        private final int _batchRequestNum;
    }

    public
    CollocatedRequestHandler(Reference ref, Ice.ObjectAdapter adapter)
    {
        _reference = ref;
        _dispatcher = ref.getInstance().initializationData().dispatcher != null;
        _adapter = (Ice.ObjectAdapterI)adapter;
        _response = _reference.getMode() == Reference.ModeTwoway;

        _logger = _reference.getInstance().initializationData().logger; // Cached for better performance.
        _traceLevels = _reference.getInstance().traceLevels(); // Cached for better performance.
        _requestId = 0;
    }

    @Override
    public RequestHandler
    update(RequestHandler previousHandler, RequestHandler newHandler)
    {
        return previousHandler == this ? newHandler : this;
    }

    @Override
    public int
    sendAsyncRequest(ProxyOutgoingAsyncBase outAsync)
    {
        return outAsync.invokeCollocated(this);
    }

    @Override
    synchronized public void
    asyncRequestCanceled(OutgoingAsyncBase outAsync, Ice.LocalException ex)
    {
        Integer requestId = _sendAsyncRequests.remove(outAsync);
        if(requestId != null)
        {
            if(requestId > 0)
            {
                _asyncRequests.remove(requestId);
            }
            if(outAsync.completed(ex))
            {
                outAsync.invokeCompletedAsync();
            }
            _adapter.decDirectCount(); // invokeAll won't be called, decrease the direct count.
            return;
        }

        if(outAsync instanceof OutgoingAsync)
        {
            OutgoingAsync o = (OutgoingAsync)outAsync;
            assert(o != null);
            for(java.util.Map.Entry<Integer, OutgoingAsyncBase> e : _asyncRequests.entrySet())
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
    sendResponse(int requestId, final Ice.OutputStream os, byte status, boolean amd)
    {
        OutgoingAsyncBase outAsync = null;
        synchronized(this)
        {
            assert(_response);

            if(_traceLevels.protocol >= 1)
            {
                fillInValue(os, 10, os.size());
            }

            // Adopt the OutputStream's buffer.
            Ice.InputStream is = new Ice.InputStream(os.instance(), os.getEncoding(), os.getBuffer(), true);

            is.pos(Protocol.replyHdr.length + 4);

            if(_traceLevels.protocol >= 1)
            {
                TraceUtil.traceRecv(is, _logger, _traceLevels);
            }

            outAsync = _asyncRequests.remove(requestId);
            if(outAsync != null && !outAsync.completed(is))
            {
                outAsync = null;
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
                outAsync.invokeCompletedAsync();
            }
            else
            {
                outAsync.invokeCompleted();
            }
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
    systemException(int requestId, Ice.SystemException ex, boolean amd)
    {
        handleException(requestId, ex, amd);
        _adapter.decDirectCount();
        return true;
    }

    @Override
    public void
    invokeException(int requestId, Ice.LocalException ex, int batchRequestNum, boolean amd)
    {
        handleException(requestId, ex, amd);
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

    int invokeAsyncRequest(OutgoingAsyncBase outAsync, int batchRequestNum, boolean sync)
    {
        //
        // Increase the direct count to prevent the thread pool from being destroyed before
        // invokeAll is called. This will also throw if the object adapter has been deactivated.
        //
        _adapter.incDirectCount();

        int requestId = 0;
        try
        {
            synchronized(this)
            {
                outAsync.cancelable(this); // This will throw if the request is canceled

                if(_response)
                {
                    requestId = ++_requestId;
                    _asyncRequests.put(requestId, outAsync);
                }

                _sendAsyncRequests.put(outAsync, requestId);
            }
        }
        catch(Exception ex)
        {
            _adapter.decDirectCount();
            throw ex;
        }

        outAsync.attachCollocatedObserver(_adapter, requestId);

        if(!sync || !_response || _reference.getInstance().queueRequests() || _reference.getInvocationTimeout() > 0)
        {
            _adapter.getThreadPool().dispatch(new InvokeAllAsync(outAsync,
                                                                 outAsync.getOs(),
                                                                 requestId,
                                                                 batchRequestNum));
        }
        else if(_dispatcher)
        {
            _adapter.getThreadPool().dispatchFromThisThread(new InvokeAllAsync(outAsync,
                                                                               outAsync.getOs(),
                                                                               requestId,
                                                                               batchRequestNum));
        }
        else // Optimization: directly call invokeAll if there's no dispatcher.
        {
            if(sentAsync(outAsync))
            {
                invokeAll(outAsync.getOs(), requestId, batchRequestNum);
            }
        }
        return AsyncStatus.Queued;
    }

    private boolean
    sentAsync(final OutgoingAsyncBase outAsync)
    {
        synchronized(this)
        {
            if(_sendAsyncRequests.remove(outAsync) == null)
            {
                return false; // The request timed-out.
            }

            //
            // This must be called within the synchronization to
            // ensure completed(ex) can't be called concurrently if
            // the request is canceled.
            //
            if(!outAsync.sent())
            {
                return true;
            }
        }

        outAsync.invokeSent();
        return true;
    }

    private void
    invokeAll(Ice.OutputStream os, int requestId, int batchRequestNum)
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

        Ice.InputStream is = new Ice.InputStream(os.instance(), os.getEncoding(), os.getBuffer(), false);

        if(batchRequestNum > 0)
        {
            is.pos(Protocol.requestBatchHdr.length);
        }
        else
        {
            is.pos(Protocol.requestHdr.length);
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

                Incoming in = new Incoming(_reference.getInstance(), this, null, _adapter, _response, (byte)0,
                                           requestId);
                in.invoke(servantManager, is);
                --invokeNum;
            }
        }
        catch(Ice.LocalException ex)
        {
            invokeException(requestId, ex, invokeNum, false); // Fatal invocation exception
        }
        catch(ServantError ex)
        {
            //
            // ServantError is thrown when an Error has been raised by servant (or servant locator)
            // code. We've already attempted to complete the invocation and send a response.
            //
            Throwable t = ex.getCause();
            //
            // Suppress AssertionError and OutOfMemoryError, rethrow everything else.
            //
            if(!(t instanceof java.lang.AssertionError ||
                 t instanceof java.lang.OutOfMemoryError ||
                 t instanceof java.lang.StackOverflowError))
            {
                throw (java.lang.Error)t;
            }
        }
        catch(java.lang.Error ex)
        {
            //
            // An Error was raised outside of servant code (i.e., by Ice code).
            // Attempt to log the error and clean up. This may still fail
            // depending on the severity of the error.
            //
            // Note that this does NOT send a response to the client.
            //
            Ice.UnknownException uex = new Ice.UnknownException(ex);
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            ex.printStackTrace(pw);
            pw.flush();
            uex.unknown = sw.toString();
            _logger.error(uex.unknown);
            invokeException(requestId, uex, invokeNum, false);
            //
            // Suppress AssertionError and OutOfMemoryError, rethrow everything else.
            //
            if(!(ex instanceof java.lang.AssertionError ||
                 ex instanceof java.lang.OutOfMemoryError ||
                 ex instanceof java.lang.StackOverflowError))
            {
                throw ex;
            }
        }
        finally
        {
            _adapter.decDirectCount();
        }
    }

    private void
    handleException(int requestId, Ice.Exception ex, boolean amd)
    {
        if(requestId == 0)
        {
            return; // Ignore exception for oneway messages.
        }

        OutgoingAsyncBase outAsync = null;
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
            //
            // If called from an AMD dispatch, invoke asynchronously
            // the completion callback since this might be called from
            // the user code.
            //
            if(amd)
            {
                outAsync.invokeCompletedAsync();
            }
            else
            {
                outAsync.invokeCompleted();
            }
        }
    }

    private void
    fillInValue(Ice.OutputStream os, int pos, int value)
    {
        os.rewriteInt(value, pos);
    }

    private final Reference _reference;
    private final boolean _dispatcher;
    private final boolean _response;
    private final Ice.ObjectAdapterI _adapter;
    private final Ice.Logger _logger;
    private final TraceLevels _traceLevels;

    private int _requestId;

    // A map of outstanding requests that can be canceled. A request
    // can be canceled if it has an invocation timeout, or we support
    // interrupts.
    private java.util.Map<OutgoingAsyncBase, Integer> _sendAsyncRequests =
        new java.util.HashMap<OutgoingAsyncBase, Integer>();

    private java.util.Map<Integer, OutgoingAsyncBase> _asyncRequests =
        new java.util.HashMap<Integer, OutgoingAsyncBase>();
}
