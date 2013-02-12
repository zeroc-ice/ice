// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class OutgoingAsync extends Ice.AsyncResult implements OutgoingAsyncMessageCallback
{
    public OutgoingAsync(Ice.ObjectPrx prx, String operation, CallbackBase cb)
    {
        super(prx.ice_getCommunicator(), ((Ice.ObjectPrxHelperBase)prx).__reference().getInstance(), operation, cb);
        _proxy = (Ice.ObjectPrxHelperBase)prx;
        _encoding = Protocol.getCompatibleEncoding(_proxy.__reference().getEncoding());
    }

    public void __prepare(String operation, Ice.OperationMode mode, java.util.Map<String, String> ctx,
                          boolean explicitCtx)
    {
        _delegate = null;
        _cnt = 0;
        _mode = mode;
        _sentSynchronously = false;

        Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(_proxy.__reference().getProtocol()));

        if(explicitCtx && ctx == null)
        {
            ctx = _emptyContext;
        }

        _observer = ObserverHelper.get(_proxy, operation, ctx);

        //
        // Can't call async via a batch proxy.
        //
        if(_proxy.ice_isBatchOneway() || _proxy.ice_isBatchDatagram())
        {
            throw new Ice.FeatureNotSupportedException("can't send batch requests with AMI");
        }

        _os.writeBlob(IceInternal.Protocol.requestHdr);

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

        _os.writeByte((byte)mode.value());

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

    public boolean __sent(final Ice.ConnectionI connection)
    {
        synchronized(_monitor)
        {
            boolean alreadySent = (_state & Sent) != 0;
            _state |= Sent;

            if((_state & Done) == 0)
            {
                if(!_proxy.ice_isTwoway())
                {
                    if(_remoteObserver != null)
                    {
                        _remoteObserver.detach();
                        _remoteObserver = null;
                    }
                    _state |= Done | OK;
                    _os.resize(0, false); // Clear buffer now, instead of waiting for AsyncResult deallocation
                }
                else if(connection.timeout() > 0)
                {
                    assert(_timerTaskConnection == null && _timerTask == null);
                    _timerTaskConnection = connection;
                    _timerTask = new TimerTask()
                        {
                            public void
                            runTimerTask()
                            {
                                __runTimerTask();
                            }
                        };
                    _instance.timer().schedule(_timerTask, connection.timeout());
                }
            }
            _monitor.notifyAll();
            return !alreadySent; // Don't call the sent call is already sent.
        }
    }

    public void __sent()
    {
        __sentInternal();
    }

    public void __finished(Ice.LocalException exc, boolean sent)
    {
        synchronized(_monitor)
        {
            assert((_state & Done) == 0);
            if(_remoteObserver != null)
            {
                _remoteObserver.failed(exc.ice_name());
                _remoteObserver.detach();
                _remoteObserver = null;
            }
            if(_timerTaskConnection != null)
            {
                _instance.timer().cancel(_timerTask);
                _timerTaskConnection = null;
                _timerTask = null;
            }
        }

        //
        // NOTE: at this point, synchronization isn't needed, no other threads should be
        // calling on the callback.
        //

        try
        {
            int interval = handleException(exc, sent); // This will throw if the invocation can't be retried.
            if(interval > 0)
            {
                _instance.retryQueue().add(this, interval);
            }
            else
            {
                __send(false);
            }
        }
        catch(Ice.LocalException ex)
        {
            __exception(ex);
        }
    }

    public final void __finished(LocalExceptionWrapper exc)
    {
        //
        // NOTE: at this point, synchronization isn't needed, no other threads should be
        // calling on the callback. The LocalExceptionWrapper exception is only called
        // before the invocation is sent.
        //
        
        if(_remoteObserver != null)
        {
            _remoteObserver.failed(exc.get().ice_name());
            _remoteObserver.detach();
            _remoteObserver = null;
        }
        
        try
        {
            int interval = handleException(exc); // This will throw if the invocation can't be retried.
            if(interval > 0)
            {
                _instance.retryQueue().add(this, interval);
            }
            else
            {
                __send(false);
            }
        }
        catch(Ice.LocalException ex)
        {
            __exception(ex);
        }
    }

    public final void __finished(BasicStream is)
    {
        assert(_proxy.ice_isTwoway()); // Can only be called for twoways.

        byte replyStatus;
        try
        {
            synchronized(_monitor)
            {
                assert(_exception == null && (_state & Done) == 0);
                if(_remoteObserver != null)
                {
                    _remoteObserver.reply(is.size() - Protocol.headerSize - 4);
                    _remoteObserver.detach();
                    _remoteObserver = null;
                }

                if(_timerTaskConnection != null)
                {
                    _instance.timer().cancel(_timerTask);
                    _timerTaskConnection = null;
                    _timerTask = null;
                }

                if(_is == null) // _is can already be initialized if the invocation is retried
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
                            assert(false);
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

                _state |= Done;
                _os.resize(0, false); // Clear buffer now, instead of waiting for AsyncResult deallocation
                if(replyStatus == ReplyStatus.replyOK)
                {
                    _state |= OK;
                }
                _monitor.notifyAll();
            }
        }
        catch(Ice.LocalException ex)
        {
            __finished(ex, true);
            return;
        }

        assert(replyStatus == ReplyStatus.replyOK || replyStatus == ReplyStatus.replyUserException);
        __response();
    }

    public final boolean __send(boolean synchronous)
    {
        while(true)
        {
            int interval = 0;
            try
            {
                _delegate = _proxy.__getDelegate(true);
                int status = _delegate.__getRequestHandler().sendAsyncRequest(this);
                if((status & AsyncStatus.Sent) > 0)
                {
                    if(synchronous)
                    {
                        _sentSynchronously = true;
                        if((status & AsyncStatus.InvokeSentCallback) > 0)
                        {
                            __sent(); // Call from the user thread.
                        }
                    }
                    else
                    {
                        if((status & AsyncStatus.InvokeSentCallback) > 0)
                        {
                            __sentAsync(); // Call from a client thread pool thread.
                        }
                    }
                }
                break;
            }
            catch(LocalExceptionWrapper ex)
            {
                interval = handleException(ex);
            }
            catch(Ice.LocalException ex)
            {
                interval = handleException(ex, false);
            }

            if(interval > 0)
            {
                _instance.retryQueue().add(this, interval);
                return false;
            }
        }
        return _sentSynchronously;
    }

    public BasicStream
    __startWriteParams(Ice.FormatType format)
    {
        _os.startWriteEncaps(_encoding, format);
        return _os;
    }

    public void
    __endWriteParams()
    {
        _os.endWriteEncaps();
    }

    public void
    __writeEmptyParams()
    {
        _os.writeEmptyEncaps(_encoding);
    }

    public void 
    __writeParamEncaps(byte[] encaps)
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

    private int handleException(Ice.LocalException exc, boolean sent)
    {
        Ice.IntHolder interval = new Ice.IntHolder(0);
        try
        {
            //
            // A CloseConnectionException indicates graceful server shutdown, and is therefore
            // always repeatable without violating "at-most-once". That's because by sending a
            // close connection message, the server guarantees that all outstanding requests
            // can safely be repeated.
            //
            // An ObjectNotExistException can always be retried as well without violating
            // "at-most-once" (see the implementation of the checkRetryAfterException method of
            // the ProxyFactory class for the reasons why it can be useful).
            //
            if(!sent ||
               exc instanceof Ice.CloseConnectionException ||
               exc instanceof Ice.ObjectNotExistException)
            {
                throw exc;
            }

            //
            // Throw the exception wrapped in a LocalExceptionWrapper, to indicate that the
            // request cannot be resent without potentially violating the "at-most-once"
            // principle.
            //
            throw new LocalExceptionWrapper(exc, false);
        }
        catch(LocalExceptionWrapper ex)
        {
            if(_mode == Ice.OperationMode.Nonmutating || _mode == Ice.OperationMode.Idempotent)
            {
                _cnt = _proxy.__handleExceptionWrapperRelaxed(_delegate, ex, interval, _cnt, _observer);
            }
            else
            {
                _proxy.__handleExceptionWrapper(_delegate, ex, _observer);
            }
        }
        catch(Ice.LocalException ex)
        {
            _cnt = _proxy.__handleException(_delegate, ex, interval, _cnt, _observer);
        }
        return interval.value;
    }

    private int handleException(LocalExceptionWrapper ex)
    {
        Ice.IntHolder interval = new Ice.IntHolder(0);
        if(_mode == Ice.OperationMode.Nonmutating || _mode == Ice.OperationMode.Idempotent)
        {
            _cnt = _proxy.__handleExceptionWrapperRelaxed(_delegate, ex, interval, _cnt, _observer);
        }
        else
        {
            _proxy.__handleExceptionWrapper(_delegate, ex, _observer);
        }
        return interval.value;
    }

    private final void __runTimerTask()
    {
        Ice.ConnectionI connection;
        synchronized(_monitor)
        {
            connection = _timerTaskConnection;
            _timerTaskConnection = null;
            _timerTask = null;
        }

        if(connection != null)
        {
            connection.exception(new Ice.TimeoutException());
        }
    }

    protected Ice.ObjectPrxHelperBase _proxy;

    private Ice.ConnectionI _timerTaskConnection;
    private TimerTask _timerTask;

    private Ice._ObjectDel _delegate;
    private Ice.EncodingVersion _encoding;
    private int _cnt;
    private Ice.OperationMode _mode;

    private static final java.util.Map<String, String> _emptyContext = new java.util.HashMap<String, String>();
}
