// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class OutgoingAsync
{
    public
    OutgoingAsync()
    {
    }

    public abstract void ice_exception(Ice.LocalException ex);

    public final void
    __finished(BasicStream is)
    {
        synchronized(_monitor)
        {
            byte replyStatus;
            
            try
            {
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
                cleanup();
            }
        }
    }

    public final void
    __finished(Ice.LocalException exc)
    {
        synchronized(_monitor)
        {
            if(__os != null) // Don't retry if cleanup() was already called.
            {
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
                if(_mode == Ice.OperationMode.Nonmutating || _mode == Ice.OperationMode.Idempotent ||
                   exc instanceof Ice.CloseConnectionException || exc instanceof Ice.ObjectNotExistException)
                {
                    try
                    {
                        _cnt = ((Ice.ObjectPrxHelperBase)_proxy).__handleException(_delegate, exc, _cnt);
                        __send();
                        return;
                    }
                    catch(Ice.LocalException ex)
                    {
                    }
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
                cleanup();
            }
        }
    }

    public final boolean
    __timedOut()
    {
        //
        // No synchronization necessary, because
        // _absoluteTimeoutMillis is declared volatile. We cannot
        // synchronize here because otherwise there might be deadlocks
        // when Ice.ConnectionI calls back on this object with this
        // function.
        //
        if(_absoluteTimeoutMillis > 0)
        {
            return System.currentTimeMillis() >= _absoluteTimeoutMillis;
        }
        else
        {
            return false;
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
                // Can't call async via a oneway proxy.
                //
                ((Ice.ObjectPrxHelperBase)prx).__checkTwowayOnly(operation);

                _proxy = prx;
                _delegate = null;
                _cnt = 0;
                _mode = mode;

                Reference ref = ((Ice.ObjectPrxHelperBase)_proxy).__reference();
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
                    Ice.ImplicitContextI implicitContext =
                        ref.getInstance().getImplicitContext();
                    
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
        synchronized(_monitor)
        {
            try
            {
                while(true)
                {
                    Ice.BooleanHolder comp = new Ice.BooleanHolder();
                    _delegate = ((Ice.ObjectPrxHelperBase)_proxy).__getDelegate();
                    Ice.ConnectionI con = _delegate.__getConnection(comp);
                    if(con.timeout() >= 0)
                    {
                        _absoluteTimeoutMillis = System.currentTimeMillis() + con.timeout();
                    }
                    else
                    {
                        _absoluteTimeoutMillis = 0;
                    }
                    
                    try
                    {
                        con.sendAsyncRequest(__os, this, comp.value);
                        
                        //
                        // Don't do anything after sendAsyncRequest() returned
                        // without an exception.  I such case, there will be
                        // callbacks, i.e., calls to the __finished()
                        // functions. Since there is no mutex protection, we
                        // cannot modify state here and in such callbacks.
                        //
                        return;
                    }
                    catch(LocalExceptionWrapper ex)
                    {
                        ((Ice.ObjectPrxHelperBase)_proxy).__handleExceptionWrapper(_delegate, ex);
                    }
                    catch(Ice.LocalException ex)
                    {                   
                        _cnt = ((Ice.ObjectPrxHelperBase)_proxy).__handleException(_delegate, ex, _cnt);
                    }               
                }
            }
            catch(Ice.LocalException ex)
            {
                __finished(ex);
            }
        }
    }

    protected abstract void __response(boolean ok);

    private final void
    warning(java.lang.Exception ex)
    {
        if(__os != null) // Don't print anything if cleanup() was already called.
        {
            Reference ref = ((Ice.ObjectPrxHelperBase)_proxy).__reference();
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
        __is = null;
        __os = null;

        _monitor.notify();
    }

    protected BasicStream __is;
    protected BasicStream __os;

    private Ice.ObjectPrx _proxy;
    private Ice._ObjectDel _delegate;
    private int _cnt;
    private Ice.OperationMode _mode;

    //
    // Must be volatile, because we don't want to lock the monitor
    // below in __timedOut(), to avoid deadlocks.
    //
    private volatile long _absoluteTimeoutMillis;

    private final java.lang.Object _monitor = new java.lang.Object();
}
