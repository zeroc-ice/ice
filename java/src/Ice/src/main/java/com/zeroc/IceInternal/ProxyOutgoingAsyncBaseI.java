// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

//
// Base class for proxy based invocations. This class handles the
// retry for proxy invocations. It also ensures the child observer is
// correct notified of failures and make sure the retry task is
// correctly canceled when the invocation completes.
//
public abstract class ProxyOutgoingAsyncBaseI<T> extends OutgoingAsyncBaseI<T> implements ProxyOutgoingAsyncBase
{
    public boolean isBatch()
    {
        return _proxyMode == Reference.ModeBatchOneway || _proxyMode == Reference.ModeBatchDatagram;
    }

    @Override
    public com.zeroc.Ice.ObjectPrx getProxy()
    {
        return _proxy;
    }

    @Override
    public boolean completed(com.zeroc.Ice.InputStream is)
    {
        //
        // NOTE: this method is called from ConnectionI.parseMessage
        // with the connection locked. Therefore, it must not invoke
        // any user callbacks.
        //

        assert(_proxy.ice_isTwoway()); // Can only be called for twoways.

        if(_childObserver != null)
        {
            _childObserver.reply(is.size() - Protocol.headerSize - 4);
            _childObserver.detach();
            _childObserver = null;
        }

        byte replyStatus;
        try
        {
            replyStatus = is.readByte();

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
                com.zeroc.Ice.Identity id = com.zeroc.Ice.Identity.read(is, null);

                //
                // For compatibility with the old FacetPath.
                //
                String[] facetPath = is.readStringSeq();
                String facet;
                if(facetPath.length > 0)
                {
                    if(facetPath.length > 1)
                    {
                        throw new com.zeroc.Ice.MarshalException();
                    }
                    facet = facetPath[0];
                }
                else
                {
                    facet = "";
                }

                String operation = is.readString();

                com.zeroc.Ice.RequestFailedException ex = null;
                switch(replyStatus)
                {
                case ReplyStatus.replyObjectNotExist:
                {
                    ex = new com.zeroc.Ice.ObjectNotExistException();
                    break;
                }

                case ReplyStatus.replyFacetNotExist:
                {
                    ex = new com.zeroc.Ice.FacetNotExistException();
                    break;
                }

                case ReplyStatus.replyOperationNotExist:
                {
                    ex = new com.zeroc.Ice.OperationNotExistException();
                    break;
                }

                default:
                {
                    assert(false);
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
                String unknown = is.readString();

                com.zeroc.Ice.UnknownException ex = null;
                switch(replyStatus)
                {
                case ReplyStatus.replyUnknownException:
                {
                    ex = new com.zeroc.Ice.UnknownException();
                    break;
                }

                case ReplyStatus.replyUnknownLocalException:
                {
                    ex = new com.zeroc.Ice.UnknownLocalException();
                    break;
                }

                case ReplyStatus.replyUnknownUserException:
                {
                    ex = new com.zeroc.Ice.UnknownUserException();
                    break;
                }

                default:
                {
                    assert(false);
                    break;
                }
                }

                ex.unknown = unknown;
                throw ex;
            }

            default:
            {
                throw new com.zeroc.Ice.UnknownReplyStatusException();
            }
            }

            return finished(replyStatus == ReplyStatus.replyOK);
        }
        catch(com.zeroc.Ice.Exception ex)
        {
            return completed(ex);
        }
    }

    @Override
    public boolean completed(com.zeroc.Ice.Exception exc)
    {
        if(_childObserver != null)
        {
            _childObserver.failed(exc.ice_id());
            _childObserver.detach();
            _childObserver = null;
        }

        //
        // NOTE: at this point, synchronization isn't needed, no other threads should be
        // calling on the callback.
        //
        try
        {
            //
            // It's important to let the retry queue do the retry even if
            // the retry interval is 0. This method can be called with the
            // connection locked so we can't just retry here.
            //
            _instance.retryQueue().add(this, handleException(exc));
            return false;
        }
        catch(com.zeroc.Ice.Exception ex)
        {
            return finished(ex); // No retries, we're done
        }
    }

    @Override
    public void retryException(com.zeroc.Ice.Exception ex)
    {
        try
        {
            //
            // It's important to let the retry queue do the retry. This is
            // called from the connect request handler and the retry might
            // require could end up waiting for the flush of the
            // connection to be done.
            //
            _proxy._updateRequestHandler(_handler, null); // Clear request handler and always retry.
            _instance.retryQueue().add(this, 0);
        }
        catch(com.zeroc.Ice.Exception exc)
        {
            if(completed(exc))
            {
                invokeCompletedAsync();
            }
        }
    }

    @Override
    public void retry()
    {
        invokeImpl(false);
    }

    public void cancelable(final CancellationHandler handler)
    {
        if(_proxy._getReference().getInvocationTimeout() == -2 && _cachedConnection != null)
        {
            final int timeout = _cachedConnection.timeout();
            if(timeout > 0)
            {
                _timerFuture = _instance.timer().schedule(
                    new Runnable()
                    {
                        @Override
                        public void run()
                        {
                            cancel(new com.zeroc.Ice.ConnectionTimeoutException());
                        }
                    }, timeout, java.util.concurrent.TimeUnit.MILLISECONDS);
            }
        }
        super.cancelable(handler);
    }

    @Override
    public void abort(com.zeroc.Ice.Exception ex)
    {
        assert(_childObserver == null);
        if(finished(ex))
        {
            invokeCompletedAsync();
        }
        else if(ex instanceof com.zeroc.Ice.CommunicatorDestroyedException)
        {
            //
            // If it's a communicator destroyed exception, don't swallow
            // it but instead notify the user thread. Even if no callback
            // was provided.
            //
            throw ex;
        }
    }

    protected ProxyOutgoingAsyncBaseI(com.zeroc.Ice._ObjectPrxI prx, String op)
    {
        super(prx.ice_getCommunicator(), prx._getReference().getInstance(), op);
        _proxy = prx;
        _mode = com.zeroc.Ice.OperationMode.Normal;
        _cnt = 0;
        _sent = false;
        _proxyMode = _proxy._getReference().getMode();
    }

    protected ProxyOutgoingAsyncBaseI(com.zeroc.Ice._ObjectPrxI prx, String op, com.zeroc.Ice.OutputStream os)
    {
        super(prx.ice_getCommunicator(), prx._getReference().getInstance(), op, os);
        _proxy = prx;
        _mode = com.zeroc.Ice.OperationMode.Normal;
        _cnt = 0;
        _sent = false;
        _proxyMode = _proxy._getReference().getMode();
    }

    @Override
    protected boolean needCallback()
    {
        return !isBatch();
    }

    protected void invokeImpl(boolean userThread)
    {
        try
        {
            if(userThread)
            {
                int invocationTimeout = _proxy._getReference().getInvocationTimeout();
                if(invocationTimeout > 0)
                {
                    _timerFuture = _instance.timer().schedule(
                        new Runnable()
                        {
                            @Override
                            public void run()
                            {
                                cancel(new com.zeroc.Ice.InvocationTimeoutException());
                            }
                        }, invocationTimeout, java.util.concurrent.TimeUnit.MILLISECONDS);
                }
            }
            else // If not called from the user thread, it's called from the retry queue
            {
                if(_observer != null)
                {
                    _observer.retried();
                }
            }

            while(true)
            {
                try
                {
                    _sent = false;
                    _handler = null;
                    _handler = _proxy._getRequestHandler();
                    int status = _handler.sendAsyncRequest(this);
                    if((status & AsyncStatus.Sent) > 0)
                    {
                        if(userThread)
                        {
                            _sentSynchronously = true;
                            if((status & AsyncStatus.InvokeSentCallback) > 0)
                            {
                                invokeSent(); // Call the sent callback from the user thread.
                            }
                        }
                        else
                        {
                            if((status & AsyncStatus.InvokeSentCallback) > 0)
                            {
                                invokeSentAsync(); // Call the sent callback from a client thread pool thread.
                            }
                        }
                    }
                    return; // We're done!
                }
                catch(RetryException ex)
                {
                    _proxy._updateRequestHandler(_handler, null); // Clear request handler and always retry.
                }
                catch(com.zeroc.Ice.Exception ex)
                {
                    if(_childObserver != null)
                    {
                        _childObserver.failed(ex.ice_id());
                        _childObserver.detach();
                        _childObserver = null;
                    }
                    final int interval = handleException(ex);
                    if(interval > 0)
                    {
                        _instance.retryQueue().add(this, interval);
                        return;
                    }
                    else if(_observer != null)
                    {
                        _observer.retried();
                    }
                }
            }
        }
        catch(com.zeroc.Ice.Exception ex)
        {
            //
            // If called from the user thread we re-throw, the exception
            // will be catch by the caller and abort() will be called.
            //
            if(userThread)
            {
                throw ex;
            }
            else if(finished(ex)) // No retries, we're done
            {
                invokeCompletedAsync();
            }
        }
    }

    @Override
    protected boolean sent(boolean done)
    {
        _sent = true;
        if(done)
        {
            if(_timerFuture != null)
            {
                _timerFuture.cancel(false);
                _timerFuture = null;
            }
        }
        return super.sent(done);
    }

    @Override
    protected boolean finished(com.zeroc.Ice.Exception ex)
    {
        if(_timerFuture != null)
        {
            _timerFuture.cancel(false);
            _timerFuture = null;
        }
        return super.finished(ex);
    }

    @Override
    protected boolean finished(boolean ok)
    {
        if(_timerFuture != null)
        {
            _timerFuture.cancel(false);
            _timerFuture = null;
        }
        return super.finished(ok);
    }

    protected int handleException(com.zeroc.Ice.Exception exc)
    {
        Holder<Integer> interval = new Holder<>();
        _cnt = _proxy._handleException(exc, _handler, _mode, _sent, interval, _cnt);
        return interval.value;
    }

    protected void prepare(java.util.Map<String, String> ctx)
    {
        Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(_proxy._getReference().getProtocol()));

        _observer = ObserverHelper.get(_proxy, _operation, ctx == null ? _emptyContext : ctx);

        switch(_proxyMode)
        {
            case Reference.ModeTwoway:
            case Reference.ModeOneway:
            case Reference.ModeDatagram:
            {
                _os.writeBlob(Protocol.requestHdr);
                break;
            }

            case Reference.ModeBatchOneway:
            case Reference.ModeBatchDatagram:
            {
                _proxy._getBatchRequestQueue().prepareBatchRequest(_os);
                break;
            }
        }

        Reference ref = _proxy._getReference();

        ref.getIdentity().write(_os);

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

        _os.writeString(_operation);

        _os.writeByte((byte)_mode.value());

        if(ctx != com.zeroc.Ice.ObjectPrx.noExplicitContext)
        {
            //
            // Explicit context
            //
            com.zeroc.Ice.ContextHelper.write(_os, ctx);
        }
        else
        {
            //
            // Implicit context
            //
            com.zeroc.Ice.ImplicitContextI implicitContext = ref.getInstance().getImplicitContext();
            java.util.Map<String, String> prxContext = ref.getContext();

            if(implicitContext == null)
            {
                com.zeroc.Ice.ContextHelper.write(_os, prxContext);
            }
            else
            {
                implicitContext.write(prxContext, _os);
            }
        }
    }

    final protected com.zeroc.Ice._ObjectPrxI _proxy;
    protected RequestHandler _handler;
    protected com.zeroc.Ice.OperationMode _mode;
    protected int _proxyMode;

    private java.util.concurrent.Future<?> _timerFuture;
    private int _cnt;
    private boolean _sent;

    private static final java.util.Map<String, String> _emptyContext = new java.util.HashMap<>();
}
