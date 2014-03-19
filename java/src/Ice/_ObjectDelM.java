// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

import Ice.Instrumentation.InvocationObserver;

public class _ObjectDelM implements _ObjectDel
{
    public boolean
    ice_isA(String __id, java.util.Map<String, String> __context, InvocationObserver __observer)
        throws IceInternal.LocalExceptionWrapper
    {
        IceInternal.Outgoing __og = __handler.getOutgoing("ice_isA", OperationMode.Nonmutating, __context, __observer);
        try
        {
            try
            {
                IceInternal.BasicStream __os = __og.startWriteParams(Ice.FormatType.DefaultFormat);
                __os.writeString(__id);
                __og.endWriteParams();
            }
            catch(LocalException __ex)
            {
                __og.abort(__ex);
            }
            boolean __ok = __og.invoke();
            try
            {
                if(!__ok)
                {
                    try
                    {
                        __og.throwUserException();
                    }
                    catch(UserException __ex)
                    {
                        throw new UnknownUserException(__ex.ice_name(), __ex);
                    }
                }
                IceInternal.BasicStream __is = __og.startReadParams();
                boolean __ret = __is.readBool();
                __og.endReadParams();
                return __ret;
            }
            catch(LocalException __ex)
            {
                throw new IceInternal.LocalExceptionWrapper(__ex, false);
            }
        }
        finally
        {
            __handler.reclaimOutgoing(__og);
        }
    }

    public void
    ice_ping(java.util.Map<String, String> __context, InvocationObserver __observer)
        throws IceInternal.LocalExceptionWrapper
    {
        IceInternal.Outgoing __og = __handler.getOutgoing("ice_ping", OperationMode.Nonmutating, __context, __observer);
        try
        {
            __og.writeEmptyParams();
            boolean __ok = __og.invoke();
            if(__og.hasResponse())
            {
                try
                {
                    if(!__ok)
                    {
                        try
                        {
                            __og.throwUserException();
                        }
                        catch(UserException __ex)
                        {
                            throw new UnknownUserException(__ex.ice_name(), __ex);
                        }
                    }
                    __og.readEmptyParams();
                }
                catch(LocalException __ex)
                {
                    throw new IceInternal.LocalExceptionWrapper(__ex, false);
                }
            }
        }
        finally
        {
            __handler.reclaimOutgoing(__og);
        }
    }

    public String[]
    ice_ids(java.util.Map<String, String> __context, InvocationObserver __observer)
        throws IceInternal.LocalExceptionWrapper
    {
        IceInternal.Outgoing __og = __handler.getOutgoing("ice_ids", OperationMode.Nonmutating, __context, __observer);
        try
        {
            __og.writeEmptyParams();
            boolean __ok = __og.invoke();
            try
            {
                if(!__ok)
                {
                    try
                    {
                        __og.throwUserException();
                    }
                    catch(UserException __ex)
                    {
                        throw new UnknownUserException(__ex.ice_name(), __ex);
                    }
                }
                IceInternal.BasicStream __is = __og.startReadParams();
                String[] __ret = __is.readStringSeq();
                __og.endReadParams();
                return __ret;
            }
            catch(LocalException __ex)
            {
                throw new IceInternal.LocalExceptionWrapper(__ex, false);
            }
        }
        finally
        {
            __handler.reclaimOutgoing(__og);
        }
    }

    public String
    ice_id(java.util.Map<String, String> __context, InvocationObserver __observer)
        throws IceInternal.LocalExceptionWrapper
    {
        IceInternal.Outgoing __og = __handler.getOutgoing("ice_id", OperationMode.Nonmutating, __context, __observer);
        try
        {
            __og.writeEmptyParams();
            boolean __ok = __og.invoke();
            try
            {
                if(!__ok)
                {
                    try
                    {
                        __og.throwUserException();
                    }
                    catch(UserException __ex)
                    {
                        throw new UnknownUserException(__ex.ice_name(), __ex);
                    }
                }
                IceInternal.BasicStream __is = __og.startReadParams();
                String __ret = __is.readString();
                __og.endReadParams();
                return __ret;
            }
            catch(LocalException __ex)
            {
                throw new IceInternal.LocalExceptionWrapper(__ex, false);
            }
        }
        finally
        {
            __handler.reclaimOutgoing(__og);
        }
    }

    public boolean
    ice_invoke(String operation, OperationMode mode, byte[] inParams, ByteSeqHolder outParams, 
               java.util.Map<String, String> __context, InvocationObserver __observer)
        throws IceInternal.LocalExceptionWrapper
    {
        IceInternal.Outgoing __og = __handler.getOutgoing(operation, mode, __context, __observer);
        try
        {
            __og.writeParamEncaps(inParams);
            boolean ok = __og.invoke();
            if(__handler.getReference().getMode() == IceInternal.Reference.ModeTwoway)
            {
                try
                {
                    if(outParams != null) 
                    {
                        outParams.value = __og.readParamEncaps();
                    }
                }
                catch(LocalException __ex)
                {
                    throw new IceInternal.LocalExceptionWrapper(__ex, false);
                }
            }
            return ok;
        }
        finally
        {
            __handler.reclaimOutgoing(__og);
        }
    }

    public void
    ice_flushBatchRequests(InvocationObserver observer)
    {
        IceInternal.BatchOutgoing out = new IceInternal.BatchOutgoing(__handler, observer);
        out.invoke();
    }

    public IceInternal.RequestHandler
    __getRequestHandler()
    {
        return __handler;
    }

    public void
    __setRequestHandler(IceInternal.RequestHandler handler)
    {
        __handler = handler;
    }

    //
    // Only for use by ObjectPrx
    //
    final void
    __copyFrom(_ObjectDelM from)
    {
        //
        // No need to synchronize "from", as the delegate is immutable
        // after creation.
        //

        //
        // No need to synchronize, as this operation is only called
        // upon initialization.
        //

        assert(__handler == null);

        __handler = from.__handler;
    }

    protected IceInternal.RequestHandler __handler;

    public void
    setup(IceInternal.Reference ref, Ice.ObjectPrx proxy, boolean async)
    {
        //
        // No need to synchronize, as this operation is only called
        // upon initialization.
        //

        assert(__handler == null);

        if(async)
        {
            IceInternal.ConnectRequestHandler handler = new IceInternal.ConnectRequestHandler(ref, proxy, this);
            __handler = handler.connect();
        }
        else
        {
            __handler = new IceInternal.ConnectionRequestHandler(ref, proxy);
        }
    }
}
