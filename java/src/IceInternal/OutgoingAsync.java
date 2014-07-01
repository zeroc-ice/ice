// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class OutgoingAsync extends Ice.AsyncResult implements OutgoingAsyncMessageCallback, TimerTask
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
        _handler = null;
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

    @Override public Ice.ObjectPrx 
    getProxy()
    {
        return _proxy;
    }

    public int
    __send(Ice.ConnectionI connection, boolean compress, boolean response)
        throws RetryException
    {
        return connection.sendAsyncRequest(this, compress, response);
    }

    public int
    __invokeCollocated(CollocatedRequestHandler handler)
    {
        return handler.invokeAsyncRequest(this);
    }

    public boolean 
    __sent()
    {
        synchronized(_monitor)
        {
            boolean alreadySent = (_state & Sent) != 0;
            _state |= Sent;
            
            assert((_state & Done) == 0);

            if(!_proxy.ice_isTwoway())
            {
                if(_childObserver != null)
                {
                    _childObserver.detach();
                    _childObserver = null;
                }
                if(_timeoutRequestHandler != null)
                {
                    _instance.timer().cancel(this);
                    _timeoutRequestHandler = null;
                }
                _state |= Done | OK;
                //_os.resize(0, false); // Don't clear the buffer now, it's needed for the collocation optimization
            }
            _monitor.notifyAll();
            return !alreadySent; // Don't call the sent call is already sent.
        }
    }

    public void 
    __invokeSent()
    {
        __invokeSentInternal();
    }

    public void 
    __finished(Ice.Exception exc, boolean sent)
    {
        synchronized(_monitor)
        {
            assert((_state & Done) == 0);
            if(_childObserver != null)
            {
                _childObserver.failed(exc.ice_name());
                _childObserver.detach();
                _childObserver = null;
            }
            if(_timeoutRequestHandler != null)
            {
                _instance.timer().cancel(this);
                _timeoutRequestHandler = null;
            }
        }

        //
        // NOTE: at this point, synchronization isn't needed, no other threads should be
        // calling on the callback.
        //
        try
        {
            if(!handleException(exc, sent))
            {
                return; // Can't be retried immediately.
            }
            
            __invoke(false); // Retry the invocation
        }
        catch(Ice.Exception ex)
        {
            __invokeException(ex);
        }
    }

    public final void
    __finished(BasicStream is)
    {
        assert(_proxy.ice_isTwoway()); // Can only be called for twoways.

        byte replyStatus;
        try
        {
            synchronized(_monitor)
            {
                assert(_exception == null && (_state & Done) == 0);
                if(_childObserver != null)
                {
                    _childObserver.reply(is.size() - Protocol.headerSize - 4);
                    _childObserver.detach();
                    _childObserver = null;
                }

                if(_timeoutRequestHandler != null)
                {
                    _instance.timer().cancel(this);
                    _timeoutRequestHandler = null;
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
        __invokeCompleted();
    }

    public final boolean 
    __invoke(boolean synchronous)
    {
        while(true)
        {
            try
            {
                _handler = _proxy.__getRequestHandler(true);
                int status = _handler.sendAsyncRequest(this);
                if((status & AsyncStatus.Sent) > 0)
                {
                    if(synchronous)
                    {
                        _sentSynchronously = true;
                        if((status & AsyncStatus.InvokeSentCallback) > 0)
                        {
                            __invokeSent(); // Call from the user thread.
                        }
                    }
                    else
                    {
                        if((status & AsyncStatus.InvokeSentCallback) > 0)
                        {
                            __invokeSentAsync(); // Call from a client thread pool thread.
                        }
                    }
                }

                if(_proxy.ice_isTwoway() || (status & AsyncStatus.Sent) == 0)
                {
                    synchronized(_monitor)
                    {
                        if((_state & Done) == 0)
                        {
                            int invocationTimeout = _handler.getReference().getInvocationTimeout();
                            if(invocationTimeout > 0)
                            {
                                _instance.timer().schedule(this, invocationTimeout);
                                _timeoutRequestHandler = _handler;
                            }
                        }
                    }
                }
                break;
            }
            catch(RetryException ex)
            {
                _proxy.__setRequestHandler(_handler, null); // Clear request handler and retry.
            }
            catch(Ice.Exception ex)
            {
                if(!handleException(ex, false)) // This will throw if the invocation can't be retried.
                {
                    break; // Can't be retried immediately.
                }
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

    BasicStream
    __getIs()
    {
        return _is;
    }

    public void 
    runTimerTask()
    {
        __runTimerTask();
    }

    private boolean 
    handleException(Ice.Exception exc, boolean sent)
    {
        try
        {
            Ice.IntHolder interval = new Ice.IntHolder();
            _cnt = _proxy.__handleException(exc, _handler, _mode, sent, interval, _cnt);
            if(_observer != null)
            {
                _observer.retried(); // Invocation is being retried.
            }
            if(interval.value > 0)
            {
                _instance.retryQueue().add(this, interval.value);
                return false; // Don't retry immediately, the retry queue will take care of the retry.
            }
            else
            {
                return true; // Retry immediately.
            }
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
    private Ice.EncodingVersion _encoding;
    private int _cnt;
    private Ice.OperationMode _mode;

    private static final java.util.Map<String, String> _emptyContext = new java.util.HashMap<String, String>();
}
