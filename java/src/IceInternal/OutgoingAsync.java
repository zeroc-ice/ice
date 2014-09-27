// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class OutgoingAsync extends OutgoingAsyncBase implements OutgoingAsyncMessageCallback
{
    public OutgoingAsync(Ice.ObjectPrx prx, String operation, CallbackBase cb)
    {
        super(prx.ice_getCommunicator(), ((Ice.ObjectPrxHelperBase) prx).__reference().getInstance(), operation, cb);
        _proxy = (Ice.ObjectPrxHelperBase) prx;
        _encoding = Protocol.getCompatibleEncoding(_proxy.__reference().getEncoding());
    }

    public OutgoingAsync(Ice.ObjectPrx prx, String operation, CallbackBase cb, IceInternal.BasicStream is,
            IceInternal.BasicStream os)
    {
        super(prx.ice_getCommunicator(), ((Ice.ObjectPrxHelperBase) prx).__reference().getInstance(), operation, cb,
                is, os);
        _proxy = (Ice.ObjectPrxHelperBase) prx;
        _encoding = Protocol.getCompatibleEncoding(_proxy.__reference().getEncoding());
    }

    public void prepare(String operation, Ice.OperationMode mode, java.util.Map<String, String> ctx,
                        boolean explicitCtx, boolean synchronous)
    {
        _handler = null;
        _cnt = 0;
        _sent = false;
        _mode = mode;
        _sentSynchronously = false;
        _synchronous = synchronous;

        Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(_proxy.__reference().getProtocol()));

        if(explicitCtx && ctx == null)
        {
            ctx = _emptyContext;
        }

        _observer = ObserverHelper.get(_proxy, operation, ctx);

        switch(_proxy.__reference().getMode())
        {
            case Reference.ModeTwoway:
            case Reference.ModeOneway:
            case Reference.ModeDatagram:
            {
                _os.writeBlob(IceInternal.Protocol.requestHdr);
                break;
            }

            case Reference.ModeBatchOneway:
            case Reference.ModeBatchDatagram:
            {
                while(true)
                {
                    try
                    {
                        _handler = _proxy.__getRequestHandler();
                        _handler.prepareBatchRequest(_os);
                        break;
                    }
                    catch(RetryException ex)
                    {
                        // Clear request handler and retry.
                        _proxy.__setRequestHandler(_handler, null);
                    }
                    catch(Ice.LocalException ex)
                    {
                        if(_observer != null)
                        {
                            _observer.failed(ex.ice_name());
                        }
                        // Clear request handler
                        _proxy.__setRequestHandler(_handler, null);
                        _handler = null;
                        throw ex;
                    }
                }
                break;
            }
        }

        Reference ref = _proxy.__reference();

        ref.getIdentity().__write(_os);

        //
        // For compatibility with the old FacetPath.
        //
        String facet = ref.getFacet();
        if(facet == null || facet.length() == 0)
        {
            _os.writeStringSeq(null);
        }
        else
        {
            String[] facetPath = { facet };
            _os.writeStringSeq(facetPath);
        }

        _os.writeString(operation);

        _os.writeByte((byte) mode.value());

        if(ctx != null)
        {
            //
            // Explicit context
            //
            Ice.ContextHelper.write(_os, ctx);
        }
        else
        {
            //
            // Implicit context
            //
            Ice.ImplicitContextI implicitContext = ref.getInstance().getImplicitContext();
            java.util.Map<String, String> prxContext = ref.getContext();

            if(implicitContext == null)
            {
                Ice.ContextHelper.write(_os, prxContext);
            }
            else
            {
                implicitContext.write(prxContext, _os);
            }
        }
    }

    @Override
    public Ice.ObjectPrx getProxy()
    {
        return _proxy;
    }

    @Override
    public int send(Ice.ConnectionI connection, boolean compress, boolean response) throws RetryException
    {
        _cachedConnection = connection;
        return connection.sendAsyncRequest(this, compress, response);
    }

    @Override
    public int invokeCollocated(CollocatedRequestHandler handler)
    {
        // The BasicStream cannot be cached if the proxy is not a twoway or there is an invocation timeout set.
        if(!_proxy.ice_isTwoway() || _proxy.__reference().getInvocationTimeout() > 0)
        {
            // Disable caching by marking the streams as cached!
            _state |= StateCachedBuffers;
        }
        handler.invokeAsyncRequest(this, _synchronous);
        return AsyncStatus.Queued;
    }

    @Override
    public boolean sent()
    {
        synchronized(_monitor)
        {
            boolean alreadySent = (_state & StateSent) != 0;
            _state |= StateSent;
            _sent = true;

            assert ((_state & StateDone) == 0);

            if(!_proxy.ice_isTwoway())
            {
                if(_childObserver != null)
                {
                    _childObserver.detach();
                    _childObserver = null;
                }
                if(_observer != null && (_callback == null || !_callback.__hasSentCallback()))
                {
                    _observer.detach();
                    _observer = null;
                }
                if(_timeoutRequestHandler != null)
                {
                    _future.cancel(false);
                    _future = null;
                    _timeoutRequestHandler = null;
                }
                _state |= StateDone | StateOK;
                // _os.resize(0, false); // Don't clear the buffer now, it's
                // needed for the collocation optimization
            }
            _monitor.notifyAll();

            // Don't call the sent call is already sent.
            return !alreadySent && _callback != null && _callback.__hasSentCallback();
        }
    }

    @Override
    public void invokeSent()
    {
        invokeSentInternal();
    }

    @Override
    public void finished(Ice.Exception exc)
    {
        synchronized(_monitor)
        {
            assert ((_state & StateDone) == 0);
            if(_childObserver != null)
            {
                _childObserver.failed(exc.ice_name());
                _childObserver.detach();
                _childObserver = null;
            }
            if(_timeoutRequestHandler != null)
            {
                _future.cancel(false);
                _future = null;
                _timeoutRequestHandler = null;
            }
        }

        //
        // NOTE: at this point, synchronization isn't needed, no other threads
        // should be calling on the callback.
        //
        try
        {
            handleException(exc);
        }
        catch(Ice.Exception ex)
        {
            invokeException(ex);
        }
    }

    @Override
    public void processRetry(boolean destroyed)
    {
        if(destroyed)
        {
            invokeExceptionAsync(new Ice.CommunicatorDestroyedException());
        }
        else
        {
            try
            {
                invoke(false);
            }
            catch(Ice.LocalException ex)
            {
                invokeExceptionAsync(ex);
            }
        }
    }

    @Override
    public void dispatchInvocationCancel(final Ice.LocalException ex, ThreadPool threadPool, Ice.Connection connection)
    {
        threadPool.dispatch(new DispatchWorkItem(connection)
        {
            @Override
            public void run()
            {
                OutgoingAsync.this.finished(ex);
            }
        });
    }

    public final boolean finished(BasicStream is)
    {
        //
        // NOTE: this method is called from ConnectionI.parseMessage
        // with the connection locked. Therefore, it must not invoke
        // any user callbacks.
        //

        assert (_proxy.ice_isTwoway()); // Can only be called for twoways.

        byte replyStatus;
        try
        {
            synchronized(_monitor)
            {
                assert (_exception == null && (_state & StateDone) == 0);
                if(_childObserver != null)
                {
                    _childObserver.reply(is.size() - Protocol.headerSize - 4);
                    _childObserver.detach();
                    _childObserver = null;
                }

                if(_timeoutRequestHandler != null)
                {
                    _future.cancel(false);
                    _future = null;
                    _timeoutRequestHandler = null;
                }

                // _is can already be initialized if the invocation is retried
                if(_is == null)
                {
                    _is = new IceInternal.BasicStream(_instance, IceInternal.Protocol.currentProtocolEncoding);
                }
                _is.swap(is);
                replyStatus = _is.readByte();

                switch(replyStatus)
                {
                    case ReplyStatus.replyOK:
                    {
                        break;
                    }

                    case ReplyStatus.replyUserException:
                    {
                        if(_observer != null)
                        {
                            _observer.userException();
                        }
                        break;
                    }

                    case ReplyStatus.replyObjectNotExist:
                    case ReplyStatus.replyFacetNotExist:
                    case ReplyStatus.replyOperationNotExist:
                    {
                        Ice.Identity id = new Ice.Identity();
                        id.__read(_is);

                        //
                        // For compatibility with the old FacetPath.
                        //
                        String[] facetPath = _is.readStringSeq();
                        String facet;
                        if(facetPath.length > 0)
                        {
                            if(facetPath.length > 1)
                            {
                                throw new Ice.MarshalException();
                            }
                            facet = facetPath[0];
                        }
                        else
                        {
                            facet = "";
                        }

                        String operation = _is.readString();

                        Ice.RequestFailedException ex = null;
                        switch(replyStatus)
                        {
                            case ReplyStatus.replyObjectNotExist:
                            {
                                ex = new Ice.ObjectNotExistException();
                                break;
                            }

                            case ReplyStatus.replyFacetNotExist:
                            {
                                ex = new Ice.FacetNotExistException();
                                break;
                            }

                            case ReplyStatus.replyOperationNotExist:
                            {
                                ex = new Ice.OperationNotExistException();
                                break;
                            }

                            default:
                            {
                                assert (false);
                                break;
                            }
                        }

                        ex.id = id;
                        ex.facet = facet;
                        ex.operation = operation;
                        throw ex;
                    }

                    case ReplyStatus.replyUnknownException:
                    case ReplyStatus.replyUnknownLocalException:
                    case ReplyStatus.replyUnknownUserException:
                    {
                        String unknown = _is.readString();

                        Ice.UnknownException ex = null;
                        switch(replyStatus)
                        {
                            case ReplyStatus.replyUnknownException:
                            {
                                ex = new Ice.UnknownException();
                                break;
                            }

                            case ReplyStatus.replyUnknownLocalException:
                            {
                                ex = new Ice.UnknownLocalException();
                                break;
                            }

                            case ReplyStatus.replyUnknownUserException:
                            {
                                ex = new Ice.UnknownUserException();
                                break;
                            }

                            default:
                            {
                                assert (false);
                                break;
                            }
                        }

                        ex.unknown = unknown;
                        throw ex;
                    }

                    default:
                    {
                        throw new Ice.UnknownReplyStatusException();
                    }
                }

                if(replyStatus == ReplyStatus.replyOK)
                {
                    _state |= StateOK;
                }
                _state |= StateDone;
                _monitor.notifyAll();

                if(_callback == null)
                {
                    if(_observer != null)
                    {
                        _observer.detach();
                        _observer = null;
                    }
                    return false;
                }
                return true;
            }
        }
        catch(Ice.Exception exc)
        {
            //
            // We don't call finished(exc) here because we don't want
            // to invoke the completion callback. The completion
            // callback is invoked by the connection is this method
            // returns true.
            //
            try
            {
                handleException(exc);
                return false;
            }
            catch(Ice.LocalException ex)
            {
                synchronized(_monitor)
                {
                    _state |= StateDone;
                    _exception = ex;
                    _monitor.notifyAll();

                    if(_callback == null)
                    {
                        if(_observer != null)
                        {
                            _observer.detach();
                            _observer = null;
                        }
                        return false;
                    }
                    return true;
                }
            }
        }
    }

    public final boolean invoke(boolean synchronous)
    {
        int mode = _proxy.__reference().getMode();
        if(mode == Reference.ModeBatchOneway || mode == Reference.ModeBatchDatagram)
        {
            _state |= StateDone | StateOK;
            _handler.finishBatchRequest(_os);
            if(_observer != null)
            {
                _observer.detach();
                _observer = null;
            }
            return true;
        }

        while(true)
        {
            try
            {
                _sent = false;
                _handler = _proxy.__getRequestHandler();
                int status = _handler.sendAsyncRequest(this);
                if((status & AsyncStatus.Sent) > 0)
                {
                    if(synchronous)
                    {
                        _sentSynchronously = true;
                        if((status & AsyncStatus.InvokeSentCallback) > 0)
                        {
                            invokeSent(); // Call from the user thread.
                        }
                    }
                    else
                    {
                        if((status & AsyncStatus.InvokeSentCallback) > 0)
                        {
                            // Call from a client thread pool thread.
                            invokeSentAsync();
                        }
                    }
                }

                if(mode == IceInternal.Reference.ModeTwoway || (status & AsyncStatus.Sent) == 0)
                {
                    synchronized(_monitor)
                    {
                        if((_state & StateDone) == 0)
                        {
                            int invocationTimeout = _handler.getReference().getInvocationTimeout();
                            if(invocationTimeout > 0)
                            {
                                _future = _instance.timer().schedule(new Runnable()
                                {
                                    @Override
                                    public void run()
                                    {
                                        timeout();
                                    }
                                }, invocationTimeout, java.util.concurrent.TimeUnit.MILLISECONDS);
                                _timeoutRequestHandler = _handler;
                            }
                        }
                    }
                }
            }
            catch(RetryException ex)
            {
                // Clear request handler and retry.
                _proxy.__setRequestHandler(_handler, null);
                continue;
            }
            catch(Ice.Exception ex)
            {
                // This will throw if the invocation can't be retried.
                handleException(ex);
            }
            break;
        }
        return _sentSynchronously;
    }

    public BasicStream startWriteParams(Ice.FormatType format)
    {
        _os.startWriteEncaps(_encoding, format);
        return _os;
    }

    public void endWriteParams()
    {
        _os.endWriteEncaps();
    }

    public void writeEmptyParams()
    {
        _os.writeEmptyEncaps(_encoding);
    }

    public void writeParamEncaps(byte[] encaps)
    {
        if(encaps == null || encaps.length == 0)
        {
            _os.writeEmptyEncaps(_encoding);
        }
        else
        {
            _os.writeEncaps(encaps);
        }
    }

    public void cacheMessageBuffers()
    {
        if(_proxy.__reference().getInstance().cacheMessageBuffers() > 0)
        {
            synchronized(_monitor)
            {
                if((_state & StateCachedBuffers) > 0)
                {
                    return;
                }
                _state |= StateCachedBuffers;
            }

            if(_is != null)
            {
                _is.reset();
            }
            _os.reset();

            _proxy.cacheMessageBuffers(_is, _os);
        }
    }

    @Override
    public void invokeExceptionAsync(final Ice.Exception ex)
    {
        if((_state & StateDone) == 0 && _handler != null)
        {
            //
            // If we didn't finish a batch oneway or datagram request, we
            // must notify the connection about that we give up ownership
            // of the batch stream.
            //
            int mode = _proxy.__reference().getMode();
            if(mode == Reference.ModeBatchOneway || mode == Reference.ModeBatchDatagram)
            {
                _handler.abortBatchRequest();
            }
        }

        super.invokeExceptionAsync(ex);
    }
    
    @Override
    protected void cancelRequest()
    {
        if(_handler != null)
        {
            _handler.asyncRequestCanceled(this, new Ice.OperationInterruptedException());
        }
    }

    private void handleException(Ice.Exception exc)
    {
        try
        {
            Ice.Holder<Integer> interval = new Ice.Holder<Integer>();
            _cnt = _proxy.__handleException(exc, _handler, _mode, _sent, interval, _cnt);
            if(_observer != null)
            {
                _observer.retried(); // Invocation is being retried.
            }

            //
            // Schedule the retry. Note that we always schedule the retry
            // on the retry queue even if the invocation can be retried
            // immediately. This is required because it might not be safe
            // to retry from this thread (this is for instance called by
            // finished(BasicStream) which is called with the connection
            // locked.
            //
            _instance.retryQueue().add(this, interval.value);
        }
        catch(Ice.Exception ex)
        {
            if(_observer != null)
            {
                _observer.failed(ex.ice_name());
            }
            throw ex;
        }
    }

    protected Ice.ObjectPrxHelperBase _proxy;

    private RequestHandler _handler;
    private int _cnt;
    private Ice.EncodingVersion _encoding;
    private Ice.OperationMode _mode;
    private boolean _sent;
    //
    // If true this AMI request is being used for a generated synchronous invocation.
    //
    private boolean _synchronous;


    private static final java.util.Map<String, String> _emptyContext = new java.util.HashMap<String, String>();


}
