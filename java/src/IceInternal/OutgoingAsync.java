// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class OutgoingAsync extends OutgoingAsyncMessageCallback
{
    public final void
    __sent(final Ice.ConnectionI connection)
    {
        synchronized(__monitor)
        {
            _sent = true;

            if(!_proxy.ice_isTwoway())
            {
                __releaseCallback();
            }
            else if(_response)
            {
                __monitor.notifyAll(); // If the response was already received notify finished() which is waiting.
            }
            else if(connection.timeout() >= 0)
            {
                assert(_timerTask == null);
                _timerTask = new TimerTask()
                {
                    public void
                    runTimerTask()
                    {
                        __runTimerTask(connection);
                    }
                };
                _proxy.__reference().getInstance().timer().schedule(_timerTask, connection.timeout());
            }
        }
    }

    public final void
    __finished(BasicStream is)
    {
        assert(_proxy.ice_isTwoway()); // Can only be called for twoways.

        byte replyStatus;
        try
        {
            synchronized(__monitor)
            {
                assert(__os != null);
                _response = true;

                if(_timerTask != null && _proxy.__reference().getInstance().timer().cancel(_timerTask))
                {
                    _timerTask = null; // Timer cancelled.
                }

                while(!_sent || _timerTask != null)
                {
                    try
                    {
                        __monitor.wait();
                    }
                    catch(java.lang.InterruptedException ex)
                    {
                    }
                }

                __is.swap(is);
                replyStatus = __is.readByte();

                switch(replyStatus)
                {
                    case ReplyStatus.replyOK:
                    case ReplyStatus.replyUserException:
                    {
                        break;
                    }

                    case ReplyStatus.replyObjectNotExist:
                    case ReplyStatus.replyFacetNotExist:
                    case ReplyStatus.replyOperationNotExist:
                    {
                        Ice.Identity id = new Ice.Identity();
                        id.__read(__is);

                        //
                        // For compatibility with the old FacetPath.
                        //
                        String[] facetPath = __is.readStringSeq();
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

                        String operation = __is.readString();

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
                        String unknown = __is.readString();

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
            }
        }
        catch(Ice.LocalException ex)
        {
            __finished(ex);
            return;
        }

        assert(replyStatus == ReplyStatus.replyOK || replyStatus == ReplyStatus.replyUserException);

        try
        {
            __response(replyStatus == ReplyStatus.replyOK);
        }
        catch(java.lang.Exception ex)
        {
            __warning(ex);
            __releaseCallback();
        }
    }


    public final void
    __finished(Ice.LocalException exc)
    {
        synchronized(__monitor)
        {
            if(__os != null) // Might be called from __prepare or before __prepare
            {
                if(_timerTask != null && _proxy.__reference().getInstance().timer().cancel(_timerTask))
                {
                    _timerTask = null; // Timer cancelled.
                }

                while(_timerTask != null)
                {
                    try
                    {
                        __monitor.wait();
                    }
                    catch(java.lang.InterruptedException ex)
                    {
                    }
                }
            }
        }

        //
        // NOTE: at this point, synchronization isn't needed, no other threads should be
        // calling on the callback.
        //

        try
        {
            handleException(exc); // This will throw if the invocation can't be retried.
        }
        catch(Ice.LocalException ex)
        {
            __exception(ex);
        }
    }

    public final void
    __finished(LocalExceptionWrapper ex)
    {
        assert(__os != null && !_sent);

        //
        // NOTE: at this point, synchronization isn't needed, no other threads should be
        // calling on the callback. The LocalExceptionWrapper exception is only called
        // before the invocation is sent.
        //

        try
        {
            handleException(ex); // This will throw if the invocation can't be retried.
        }
        catch(Ice.LocalException exc)
        {
            __exception(exc);
        }
    }

    public final void
    __send(int cnt)
    {
        //
        // This method is called by the proxy to retry an invocation. It's safe to update
        // the count here without synchronization, no other threads can access this object.
        //
        _cnt = cnt;
        __send();
    }

    public final boolean
    __send()
    {
        try
        {
            _sent = false;
            _response = false;
            _delegate = _proxy.__getDelegate(true);
            _sentSynchronously = _delegate.__getRequestHandler().sendAsyncRequest(this);
        }
        catch(LocalExceptionWrapper ex)
        {
            handleException(ex);
        }
        catch(Ice.LocalException ex)
        {
            handleException(ex);
        }
        return _sentSynchronously;
    }

    protected final void
    __prepare(Ice.ObjectPrx prx, String operation, Ice.OperationMode mode, java.util.Map<String, String> context)
    {
        assert(__os != null);

        _proxy = (Ice.ObjectPrxHelperBase)prx;
        _delegate = null;
        _cnt = 0;
        _mode = mode;

        //
        // Can't call async via a batch proxy.
        //
        if(_proxy.ice_isBatchOneway() || _proxy.ice_isBatchDatagram())
        {
            throw new Ice.FeatureNotSupportedException("can't send batch requests with AMI");
        }

        __os.writeBlob(IceInternal.Protocol.requestHdr);

        Reference ref = _proxy.__reference();

        ref.getIdentity().__write(__os);

        //
        // For compatibility with the old FacetPath.
        //
        String facet = ref.getFacet();
        if(facet == null || facet.length() == 0)
        {
            __os.writeStringSeq(null);
        }
        else
        {
            String[] facetPath = { facet };
            __os.writeStringSeq(facetPath);
        }

        __os.writeString(operation);

        __os.writeByte((byte)mode.value());

        if(context != null)
        {
            //
            // Explicit context
            //
            Ice.ContextHelper.write(__os, context);
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
                Ice.ContextHelper.write(__os, prxContext);
            }
            else
            {
                implicitContext.write(prxContext, __os);
            }
        }

        __os.startWriteEncaps();
    }

    protected abstract void __response(boolean ok);

    protected void
    __throwUserException()
        throws Ice.UserException
    {
        try
        {
            __is.startReadEncaps();
            __is.throwException();
        }
        catch(Ice.UserException ex)
        {
            __is.endReadEncaps();
            throw ex;
        }
    }

    private void
    handleException(LocalExceptionWrapper ex)
    {
        if(_mode == Ice.OperationMode.Nonmutating || _mode == Ice.OperationMode.Idempotent)
        {
            _proxy.__handleExceptionWrapperRelaxed(_delegate, ex, this, _cnt);
        }
        else
        {
            _proxy.__handleExceptionWrapper(_delegate, ex, this);
        }
    }

    private void
    handleException(Ice.LocalException exc)
    {
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
            if(!_sent ||
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
                _proxy.__handleExceptionWrapperRelaxed(_delegate, ex, this, _cnt);
            }
            else
            {
                _proxy.__handleExceptionWrapper(_delegate, ex, this);
            }
        }
        catch(Ice.LocalException ex)
        {
            _proxy.__handleException(_delegate, ex, this, _cnt);
        }
    }

    private final void
    __runTimerTask(Ice.ConnectionI connection)
    {
        synchronized(__monitor)
        {
            assert(_timerTask != null && _sent); // Can only be set once the request is sent.

            if(_response) // If the response was just received, don't close the connection.
            {
                connection = null;
            }
            _timerTask = null;
            __monitor.notifyAll();
        }

        if(connection != null)
        {
            connection.exception(new Ice.TimeoutException());
        }
    }

    private boolean _sent;
    private boolean _sentSynchronously;
    private boolean _response;
    private Ice.ObjectPrxHelperBase _proxy;
    private Ice._ObjectDel _delegate;
    private int _cnt;
    private Ice.OperationMode _mode;
    private TimerTask _timerTask;
}
