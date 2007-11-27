// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class OutgoingAsync implements OutgoingAsyncMessageCallback
{
    public
    OutgoingAsync()
    {
    }

    public abstract void ice_exception(Ice.LocalException ex);

    public final BasicStream
    __os()
    {
        return __os;
    }

    public final void
    __sent(final Ice.ConnectionI connection)
    {
        synchronized(_monitor)
        {
            _sent = true;

	    if(!_proxy.ice_isTwoway())
	    {
                cleanup(); // No response expected, we're done with the OutgoingAsync.
            }
            else if(_response)
            {
                _monitor.notifyAll(); // If the response was already received notify finished() which is waiting.
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
            synchronized(_monitor)
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
                        _monitor.wait();
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
                        __is.startReadEncaps();
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
            warning(ex);
        }
        finally
        {
            synchronized(_monitor)
            {
                cleanup();
            }
        }
    }


    public final void
    __finished(Ice.LocalException exc)
    {
        boolean retry = false;
        synchronized(_monitor)
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
                        _monitor.wait();
                    }
                    catch(java.lang.InterruptedException ex)
                    {
                    }
                }
                
                //
                // A CloseConnectionException indicates graceful
                // server shutdown, and is therefore always repeatable
                // without violating "at-most-once". That's because by
                // sending a close connection message, the server
                // guarantees that all outstanding requests can safely
                // be repeated. Otherwise, we can also retry if the
                // operation mode is Nonmutating or Idempotent.
                //
                // An ObjectNotExistException can always be retried as
                // well without violating "at-most-once".
                //
                if(!_sent ||
                   _mode == Ice.OperationMode.Nonmutating || _mode == Ice.OperationMode.Idempotent ||
                   exc instanceof Ice.CloseConnectionException || exc instanceof Ice.ObjectNotExistException)
                {
                    retry = true;
                }
            }
        }

        if(retry)
        {
            try
            {
                _cnt = _proxy.__handleException(_delegate, exc, _cnt);
                __send();
                return;
            }
            catch(Ice.LocalException ex)
            {
            }
        }

        try
        {
            ice_exception(exc);
        }
        catch(java.lang.Exception ex)
        {
            warning(ex);
        }
        finally
        {
            synchronized(_monitor)
            {
                cleanup();
            }
        }
    }

    public final void
    __finished(LocalExceptionWrapper ex)
    {
        //
        // NOTE: This is called if sendRequest/sendAsyncRequest fails with
        // a LocalExceptionWrapper exception. It's not possible for the
        // timer to be set at this point because the request couldn't be
        // sent.
        //
        assert(!_sent && _timerTask == null);

        try
        {
            if(_mode == Ice.OperationMode.Nonmutating || _mode == Ice.OperationMode.Idempotent)
            {
                _cnt = _proxy.__handleExceptionWrapperRelaxed(_delegate, ex, _cnt);
            }
            else
            {
                _proxy.__handleExceptionWrapper(_delegate, ex);
            }
            __send();
        }
        catch(Ice.LocalException exc)
        {
            try
            {
                ice_exception(exc);
            }
            catch(java.lang.Exception exl)
            {
                warning(exl);
            }
            finally
            {
                synchronized(_monitor)
                {
                    cleanup();
                }
            }
        }
    }

    protected final void
    __prepare(Ice.ObjectPrx prx, String operation, Ice.OperationMode mode, java.util.Map context)
    {
        synchronized(_monitor)
        {
            try
            {
                //
                // We must first wait for other requests to finish.
                //
                while(__os != null)
                {
                    try
                    {
                        _monitor.wait();
                    }
                    catch(InterruptedException ex)
                    {
                    }
                }

                //
                // Can't call async via a batch proxy.
                //
                _proxy = (Ice.ObjectPrxHelperBase)prx;
                if(_proxy.ice_isBatchOneway() || _proxy.ice_isBatchDatagram())
                {
                    throw new Ice.FeatureNotSupportedException("can't send batch requests with AMI");
                }

                _delegate = null;
                _cnt = 0;
                _mode = mode;
                _sent = false;
		_response = false;

                Reference ref = _proxy.__reference();
                assert(__is == null);
                __is = new BasicStream(ref.getInstance());
                assert(__os == null);
                __os = new BasicStream(ref.getInstance());              

                __os.writeBlob(IceInternal.Protocol.requestHdr);

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
                    java.util.Map prxContext = ref.getContext();

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
            catch(Ice.LocalException ex)
            {
                cleanup();
                throw ex;
            }
        }
    }

    protected final void
    __send()
    {
        //
        // NOTE: no synchronization needed. At this point, no other threads can be calling on this object.
        //

        RequestHandler handler;
        try
        {
            _delegate = _proxy.__getDelegate(true);
            handler = _delegate.__getRequestHandler();
        }
        catch(Ice.LocalException ex)
        {
            __finished(ex);
            return;
        }

        _sent = false;
        _response = false;
        handler.sendAsyncRequest(this);
    }

    protected abstract void __response(boolean ok);

    private final void
    __runTimerTask(Ice.ConnectionI connection)
    {
        synchronized(_monitor)
        {
            assert(_timerTask != null && _sent); // Can only be set once the request is sent.
            
            if(_response) // If the response was just received, don't close the connection.
            {
                connection = null;
            }
            _timerTask = null;
            _monitor.notifyAll();
        }

        if(connection != null)
        {
            connection.exception(new Ice.TimeoutException());
        }
    }

    private final void
    warning(java.lang.Exception ex)
    {
        if(__os != null) // Don't print anything if cleanup() was already called.
        {
            Reference ref = _proxy.__reference();
            if(ref.getInstance().initializationData().properties.getPropertyAsIntWithDefault(
                                                                        "Ice.Warn.AMICallback", 1) > 0)
            {
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                IceUtil.OutputBase out = new IceUtil.OutputBase(pw);
                out.setUseTab(false);
                out.print("exception raised by AMI callback:\n");
                ex.printStackTrace(pw);
                pw.flush();
                ref.getInstance().initializationData().logger.warning(sw.toString());
            }
        }
    }

    private final void
    cleanup()
    {
        assert(_timerTask == null);

        __is = null;
        __os = null;

        _monitor.notify();
    }

    protected BasicStream __is;
    protected BasicStream __os;

    private boolean _sent;
    private boolean _response;
    private Ice.ObjectPrxHelperBase _proxy;
    private Ice._ObjectDel _delegate;
    private int _cnt;
    private Ice.OperationMode _mode;

    private TimerTask _timerTask;

    private final java.lang.Object _monitor = new java.lang.Object();
}
