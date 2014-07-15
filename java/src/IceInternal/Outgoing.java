// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

import Ice.Instrumentation.ChildInvocationObserver;
import Ice.Instrumentation.InvocationObserver;

public final class Outgoing implements OutgoingMessageCallback
{
    public
    Outgoing(Ice.ObjectPrxHelperBase proxy, String op, Ice.OperationMode mode, java.util.Map<String, String> context,
             boolean explicitCtx)
    {
        Reference ref = proxy.__reference();
        _state = StateUnsent;
        _sent = false;
        _proxy = proxy;
        _mode = mode;
        _handler = null;
        _observer = IceInternal.ObserverHelper.get(proxy, op, context);
        _encoding = Protocol.getCompatibleEncoding(ref.getEncoding());
        _os = new BasicStream(ref.getInstance(), Protocol.currentProtocolEncoding);

        Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(ref.getProtocol()));

        writeHeader(op, mode, context, explicitCtx);
    }

    //
    // These functions allow this object to be reused, rather than reallocated.
    //
    public void
    reset(Ice.ObjectPrxHelperBase proxy, String op, Ice.OperationMode mode, java.util.Map<String, String> context,
          boolean explicitCtx)
    {
        Reference ref = proxy.__reference();
        _state = StateUnsent;
        _exception = null;
        _sent = false;
        _proxy = proxy;
        _mode = mode;
        _handler = null;
        _observer = IceInternal.ObserverHelper.get(proxy, op, context);
        _encoding = Protocol.getCompatibleEncoding(ref.getEncoding());

        Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(ref.getProtocol()));

        writeHeader(op, mode, context, explicitCtx);
    }

    public void 
    detach()
    {
        if(_observer != null)
        {
            _observer.detach();
        }
    }

    public void
    reclaim()
    {
        if(_is != null)
        {
            _is.reset();
        }
        _os.reset();
    }

    // Returns true if ok, false if user exception.
    public boolean
    invoke()
    {
        assert(_state == StateUnsent);

        int mode = _proxy.__reference().getMode();
        if(mode == Reference.ModeBatchOneway || mode == Reference.ModeBatchDatagram)
        {
            _state = StateInProgress;
            _handler.finishBatchRequest(_os);
            return true;
        }

        int cnt = 0;
        while(true)
        {
            try
            {
                _state = StateInProgress;
                _exception = null;
                _sent = false;

                _handler = _proxy.__getRequestHandler(false);

                if(_handler.sendRequest(this)) // Request sent and no response expected, we're done.
                {
                    return true;
                }

                boolean timedOut = false;
                synchronized(this)
                {
                    //
                    // If the handler says it's not finished, we wait until we're done.
                    //

                    int invocationTimeout = _proxy.__reference().getInvocationTimeout();
                    if(invocationTimeout > 0)
                    {
                        long now = Time.currentMonotonicTimeMillis();
                        long deadline = now + invocationTimeout;
                        while((_state == StateInProgress || !_sent) && _state != StateFailed && !timedOut)
                        {
                            try
                            {
                                wait(deadline - now);
                            }
                            catch(InterruptedException ex)
                            {
                            }
                            if((_state == StateInProgress || !_sent) && _state != StateFailed)
                            {
                                now = Time.currentMonotonicTimeMillis();
                                timedOut = now >= deadline;
                            }
                        }
                    }
                    else
                    {
                        while((_state == StateInProgress || !_sent) && _state != StateFailed)
                        {
                            try
                            {
                                wait();
                            }
                            catch(InterruptedException ex)
                            {
                            }
                        }
                    }
                }
        
                if(timedOut)
                {
                    _handler.requestTimedOut(this);

                    //
                    // Wait for the exception to propagate. It's possible the request handler ignores
                    // the timeout if there was a failure shortly before requestTimedOut got called. 
                    // In this case, the exception should be set on the Outgoing.
                    //
                    synchronized(this)
                    {
                        while(_exception == null)
                        {
                            try
                            {
                                wait();
                            }
                            catch(InterruptedException ex)
                            {
                            }
                        }
                    }
                }

                if(_exception != null)
                {
                    throw (Ice.Exception)_exception.fillInStackTrace();
                }
                else
                {
                    assert(_state != StateInProgress);
                    return _state == StateOK;
                }
            }
            catch(RetryException ex)
            {
                _proxy.__setRequestHandler(_handler, null); // Clear request handler and retry.
            }
            catch(Ice.Exception ex)
            {
                try
                {
                    Ice.IntHolder interval = new Ice.IntHolder();
                    cnt = _proxy.__handleException(ex, _handler, _mode, _sent, interval, cnt);
                    if(_observer != null)
                    {
                        _observer.retried(); // Invocation is being retried.
                    }
                    if(interval.value > 0)
                    {
                        try
                        {
                            Thread.sleep(interval.value);
                        }
                        catch(InterruptedException exi)
                        {
                        }
                    }
                }
                catch(Ice.Exception exc)
                {
                    if(_observer != null)
                    {
                        _observer.failed(exc.ice_name());
                    }
                    throw exc;
                }
            }
        }
    }

    public void
    abort(Ice.LocalException ex)
    {
        assert(_state == StateUnsent);

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

        throw ex;
    }

    public boolean
    send(Ice.ConnectionI connection, boolean compress, boolean response)
        throws RetryException
    {
        return connection.sendRequest(this, compress, response);
    }

    public void
    invokeCollocated(CollocatedRequestHandler handler)
    {
        handler.invokeRequest(this);
    }

    synchronized public void
    sent()
    {
        if(_proxy.__reference().getMode() != Reference.ModeTwoway)
        {
            if(_childObserver != null)
            {
                _childObserver.detach();
                _childObserver = null;
            }
            _state = StateOK;
        }
        _sent = true;
        notify();
    }
    
    public synchronized void
    finished(BasicStream is)
    {
        assert(_proxy.__reference().getMode() == Reference.ModeTwoway); // Only for twoways.
        
        assert(_state <= StateInProgress);
        
        if(_childObserver != null)
        {
            _childObserver.reply(is.size() - Protocol.headerSize - 4);
            _childObserver.detach();
            _childObserver = null;
        }

        if(_is == null)
        {
            _is = new IceInternal.BasicStream(_proxy.__reference().getInstance(), Protocol.currentProtocolEncoding);
        }
        _is.swap(is);
        byte replyStatus = _is.readByte();
        
        switch(replyStatus)
        {
            case ReplyStatus.replyOK:
            {
                _state = StateOK; // The state must be set last, in case there is an exception.
                break;
            }
            
            case ReplyStatus.replyUserException:
            {
                if(_observer != null)
                {
                    _observer.userException();
                }
                _state = StateUserException; // The state must be set last, in case there is an exception.
                break;
            }
            
            case ReplyStatus.replyObjectNotExist:
            case ReplyStatus.replyFacetNotExist:
            case ReplyStatus.replyOperationNotExist:
            {
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
                
                ex.id = new Ice.Identity();
                ex.id.__read(_is);

                //
                // For compatibility with the old FacetPath.
                //
                String[] facetPath = _is.readStringSeq();
                if(facetPath.length > 0)
                {
                    if(facetPath.length > 1)
                    {
                        throw new Ice.MarshalException();
                    }
                    ex.facet = facetPath[0];
                }
                else
                {
                    ex.facet = "";
                }

                ex.operation = _is.readString();
                _exception = ex;

                _state = StateLocalException; // The state must be set last, in case there is an exception.
                break;
            }
            
            case ReplyStatus.replyUnknownException:
            case ReplyStatus.replyUnknownLocalException:
            case ReplyStatus.replyUnknownUserException:
            {
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
                
                ex.unknown = _is.readString();
                _exception = ex;

                _state = StateLocalException; // The state must be set last, in case there is an exception.
                break;
            }
            
            default:
            {
                _exception = new Ice.UnknownReplyStatusException();
                _state = StateLocalException;
                break;
            }
        }

        notify();
    }

    public synchronized void
    finished(Ice.Exception ex)
    {
        //assert(_state <= StateInProgress);
        if(_state > StateInProgress)
        {
            //
            // Response was already received but message
            // didn't get removed first from the connection
            // send message queue so it's possible we can be
            // notified of failures. In this case, ignore the
            // failure and assume the outgoing has been sent.
            //
            assert(_state != StateFailed);
            _sent = true;
            notify();
            return;
        }

        if(_childObserver != null)
        {
            _childObserver.failed(ex.ice_name());
            _childObserver.detach();
            _childObserver = null;
        }
        _state = StateFailed;
        _exception = ex;
        notify();
    }
    
    public BasicStream
    os()
    {
        return _os;
    }

    public BasicStream 
    startReadParams()
    {
        _is.startReadEncaps();
        return _is;
    }

    public void 
    endReadParams()
    {
        _is.endReadEncaps();
    }

    public void
    readEmptyParams()
    {
        _is.skipEmptyEncaps(null);
    }

    public byte[]
    readParamEncaps()
    {
        return _is.readEncaps(null);
    }

    public BasicStream
    startWriteParams(Ice.FormatType format)
    {
        _os.startWriteEncaps(_encoding, format);
        return _os;
    }

    public void
    endWriteParams()
    {
        _os.endWriteEncaps();
    }

    public void
    writeEmptyParams()
    {
        _os.writeEmptyEncaps(_encoding);
    }

    public void 
    writeParamEncaps(byte[] encaps)
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

    public boolean 
    hasResponse()
    {
        return _is != null && !_is.isEmpty();
    }

    public void 
    throwUserException()
        throws Ice.UserException
    {
        try
        {
            _is.startReadEncaps();
            _is.throwException(null);
        }
        catch(Ice.UserException ex)
        {
            _is.endReadEncaps();
            throw ex;
        }
    }

    public void 
    attachRemoteObserver(Ice.ConnectionInfo info, Ice.Endpoint endpt, int requestId, int size)
    {
        if(_observer != null)
        {
            _childObserver = _observer.getRemoteObserver(info, endpt, requestId, size);
            if(_childObserver != null)
            {
                _childObserver.attach();
            }
        }
    }

    public void
    attachCollocatedObserver(Ice.ObjectAdapter adapter, int requestId)
    {
        if(_observer != null)
        {
            _childObserver = _observer.getCollocatedObserver(adapter, requestId, _os.size() - Protocol.headerSize - 4);
            if(_childObserver != null)
            {
                _childObserver.attach();
            }
        }
    }

    private void
    writeHeader(String operation, Ice.OperationMode mode, java.util.Map<String, String> context, boolean explicitCtx)
    {
        if(explicitCtx && context == null)
        {
            context = _emptyContext;
        }

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
                        _handler = _proxy.__getRequestHandler(true);
                        _handler.prepareBatchRequest(_os);
                        break;
                    }
                    catch(RetryException ex)
                    {
                        _proxy.__setRequestHandler(_handler, null); // Clear request handler and retry.
                    }
                    catch(Ice.LocalException ex)
                    {
                        if(_observer != null)
                        {
                            _observer.failed(ex.ice_name());
                        }
                        _proxy.__setRequestHandler(_handler, null); // Clear request handler
                        throw ex;
                    }
                }
                break;
            }
        }

        try
        {
            _proxy.__reference().getIdentity().__write(_os);

            //
            // For compatibility with the old FacetPath.
            //
            String facet = _proxy.__reference().getFacet();
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

            if(context != null)
            {
                //
                // Explicit context
                //
                Ice.ContextHelper.write(_os, context);
            }
            else
            {
                //
                // Implicit context
                //
                Ice.ImplicitContextI implicitContext = _proxy.__reference().getInstance().getImplicitContext();
                java.util.Map<String, String> prxContext = _proxy.__reference().getContext();
                
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
        catch(Ice.LocalException ex)
        {
            abort(ex);
        }
    }

    private Ice.ObjectPrxHelperBase _proxy;
    private Ice.OperationMode _mode;
    private RequestHandler _handler;
    private Ice.EncodingVersion _encoding;
    private BasicStream _is;
    private BasicStream _os;
    private boolean _sent;
    private Ice.Exception _exception;
    
    private static final int StateUnsent = 0;
    private static final int StateInProgress = 1;
    private static final int StateOK = 2;
    private static final int StateUserException = 3;
    private static final int StateLocalException = 4;
    private static final int StateFailed = 5;
    private int _state;

    private InvocationObserver _observer;
    private ChildInvocationObserver _childObserver;

    public Outgoing next; // For use by Ice.ObjectPrxHelperBase

    private static final java.util.Map<String, String> _emptyContext = new java.util.HashMap<String, String>();
}
