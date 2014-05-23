// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

import Ice.Instrumentation.RemoteObserver;
import Ice.Instrumentation.InvocationObserver;

public final class Outgoing implements OutgoingMessageCallback
{
    public
    Outgoing(RequestHandler handler, String operation, Ice.OperationMode mode, java.util.Map<String, String> context,
             InvocationObserver observer)
        throws LocalExceptionWrapper
    {
        _state = StateUnsent;
        _exceptionWrapper = false;
        _exceptionWrapperRetry = false;
        _sent = false;
        _handler = handler;
        _observer = observer;
        _encoding = Protocol.getCompatibleEncoding(handler.getReference().getEncoding());
        _os = new BasicStream(_handler.getReference().getInstance(), Protocol.currentProtocolEncoding);

        Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(_handler.getReference().getProtocol()));

        writeHeader(operation, mode, context);
    }

    //
    // These functions allow this object to be reused, rather than reallocated.
    //
    public void
    reset(RequestHandler handler, String operation, Ice.OperationMode mode, java.util.Map<String, String> context,
          InvocationObserver observer)
        throws LocalExceptionWrapper
    {
        _state = StateUnsent;
        _exception = null;
        _exceptionWrapper = false;
        _exceptionWrapperRetry = false;
        _sent = false;
        _handler = handler;
        _observer = observer;
        _encoding = Protocol.getCompatibleEncoding(handler.getReference().getEncoding());

        Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(_handler.getReference().getProtocol()));

        writeHeader(operation, mode, context);
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
        throws LocalExceptionWrapper
    {
        assert(_state == StateUnsent);

        switch(_handler.getReference().getMode())
        {
            case Reference.ModeOneway:
            case Reference.ModeDatagram:
            case Reference.ModeTwoway:
            {
                _state = StateInProgress;

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

                    int invocationTimeout = _handler.getReference().getInvocationTimeout();
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
                    assert(_exception != null);
                }

                if(_exception != null)
                {
                    _exception.fillInStackTrace();
                    if(_exceptionWrapper)
                    {
                        throw new LocalExceptionWrapper(_exception, _exceptionWrapperRetry);
                    }
            
                    //      
                    // A CloseConnectionException indicates graceful
                    // server shutdown, and is therefore always repeatable
                    // without violating "at-most-once". That's because by
                    // sending a close connection message, the server
                    // guarantees that all outstanding requests can safely
                    // be repeated.
                    //
                    // An ObjectNotExistException can always be retried as
                    // well without violating "at-most-once" (see the
                    // implementation of the checkRetryAfterException
                    // method of the ProxyFactory class for the reasons
                    // why it can be useful).
                    //
                    if(!_sent || 
                       _exception instanceof Ice.CloseConnectionException || 
                       _exception instanceof Ice.ObjectNotExistException)
                    {
                        throw _exception;
                    }
            
                    //
                    // Throw the exception wrapped in a LocalExceptionWrapper,
                    // to indicate that the request cannot be resent without
                    // potentially violating the "at-most-once" principle.
                    //
                    throw new LocalExceptionWrapper(_exception, false);
                }
        
                assert(_state != StateInProgress);
                return _state == StateOK;
            }

            case Reference.ModeBatchOneway:
            case Reference.ModeBatchDatagram:
            {
                //
                // For batch oneways and datagrams, the same rules as for
                // regular oneways and datagrams (see comment above)
                // apply.
                //
                _state = StateInProgress;
                _handler.finishBatchRequest(_os);
                return true;
            }
        }
        
        assert(false);
        return false;
    }

    public void
    abort(Ice.LocalException ex)
        throws LocalExceptionWrapper
    {
        assert(_state == StateUnsent);

        //
        // If we didn't finish a batch oneway or datagram request, we
        // must notify the connection about that we give up ownership
        // of the batch stream.
        //
        int mode = _handler.getReference().getMode();
        if(mode == Reference.ModeBatchOneway || mode == Reference.ModeBatchDatagram)
        {
            _handler.abortBatchRequest();
        }

        throw ex;
    }

    public boolean
    send(Ice.ConnectionI connection, boolean compress, boolean response)
        throws LocalExceptionWrapper
    {
        return connection.sendRequest(this, compress, response);
    }

    synchronized public void
    sent()
    {
        if(_handler.getReference().getMode() != Reference.ModeTwoway)
        {
            if(_remoteObserver != null)
            {
                _remoteObserver.detach();
                _remoteObserver = null;
            }
            _state = StateOK;
        }
        _sent = true;
        notify();
    }
    
    public synchronized void
    finished(BasicStream is)
    {
        assert(_handler.getReference().getMode() == Reference.ModeTwoway); // Only for twoways.
        
        assert(_state <= StateInProgress);
        
        if(_remoteObserver != null)
        {
            _remoteObserver.reply(is.size() - Protocol.headerSize - 4);
            _remoteObserver.detach();
            _remoteObserver = null;
        }

        if(_is == null)
        {
            _is = new IceInternal.BasicStream(_handler.getReference().getInstance(), Protocol.currentProtocolEncoding);
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
    finished(Ice.LocalException ex, boolean sent)
    {
        assert(_state <= StateInProgress);
        if(_remoteObserver != null)
        {
            _remoteObserver.failed(ex.ice_name());
            _remoteObserver.detach();
            _remoteObserver = null;
        }
        _state = StateFailed;
        _exception = ex;
        _sent = sent;
        notify();
    }

    public synchronized void
    finished(LocalExceptionWrapper ex)
    {
        if(_remoteObserver != null)
        {
            _remoteObserver.failed(ex.get().ice_name());
            _remoteObserver.detach();
            _remoteObserver = null;
        }

        _state = StateFailed;
        _exceptionWrapper = true;
        _exceptionWrapperRetry = ex.retry();
        _exception = ex.get();
        _sent = false;
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
            _remoteObserver = _observer.getRemoteObserver(info, endpt, requestId, size);
            if(_remoteObserver != null)
            {
                _remoteObserver.attach();
            }
        }
    }

    private void
    writeHeader(String operation, Ice.OperationMode mode, java.util.Map<String, String> context)
        throws LocalExceptionWrapper
    {
        switch(_handler.getReference().getMode())
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
                _handler.prepareBatchRequest(_os);
                break;
            }
        }

        try
        {
            _handler.getReference().getIdentity().__write(_os);

            //
            // For compatibility with the old FacetPath.
            //
            String facet = _handler.getReference().getFacet();
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
                Ice.ImplicitContextI implicitContext = _handler.getReference().getInstance().getImplicitContext();
                java.util.Map<String, String> prxContext = _handler.getReference().getContext();
                
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

    private RequestHandler _handler;
    private Ice.EncodingVersion _encoding;
    private BasicStream _is;
    private BasicStream _os;
    private boolean _sent;

    private Ice.LocalException _exception;
    private boolean _exceptionWrapper;
    private boolean _exceptionWrapperRetry;
    
    private static final int StateUnsent = 0;
    private static final int StateInProgress = 1;
    private static final int StateOK = 2;
    private static final int StateUserException = 3;
    private static final int StateLocalException = 4;
    private static final int StateFailed = 5;
    private int _state;

    private InvocationObserver _observer;
    private RemoteObserver _remoteObserver;

    public Outgoing next; // For use by Ice._ObjectDelM
}
