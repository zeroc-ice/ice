// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

import Ice.Instrumentation.InvocationObserver;

/**
 * Base class for all proxies.
 **/
public class ObjectPrxHelperBase implements ObjectPrx, java.io.Serializable
{
    /**
     * Returns a hash code for this proxy.
     *
     * @return The hash code.
     **/
    public final int
    hashCode()
    {
        return _reference.hashCode();
    }

    /**
     * Returns the communicator that created this proxy.
     *
     * @return The communicator that created this proxy.
     **/
    public final Communicator
    ice_getCommunicator()
    {
        return _reference.getCommunicator();
    }

    /**
     * Returns the stringified form of this proxy.
     *
     * @return The stringified proxy.
     **/
    public final String
    toString()
    {
        return _reference.toString();
    }

    /**
     * Tests whether this proxy supports a given interface.
     *
     * @param __id The Slice type ID of an interface.
     * @return <code>true</code> if this proxy supports the specified interface; <code>false</code>, otherwise.
     **/
    public final boolean
    ice_isA(String __id)
    {
        return ice_isA(__id, null, false);
    }

    /**
     * Tests whether this proxy supports a given interface.
     *
     * @param __id The Slice type ID of an interface.
     * @param __context The <code>Context</code> map for the invocation.
     * @return <code>true</code> if this proxy supports the specified interface; <code>false</code>, otherwise.
     **/
    public final boolean
    ice_isA(String __id, java.util.Map<String, String> __context)
    {
        return ice_isA(__id, __context, true);
    }

    private static final String __ice_isA_name = "ice_isA";

    private boolean
    ice_isA(String __id, java.util.Map<String, String> __context, boolean __explicitCtx)
    {
        if(__explicitCtx && __context == null)
        {
            __context = _emptyContext;
        }

        final InvocationObserver __observer = IceInternal.ObserverHelper.get(this, __ice_isA_name, __context);
        int __cnt = 0;
        try
        {
            while(true)
            {
                _ObjectDel __del = null;
                try
                {
                    __checkTwowayOnly(__ice_isA_name);
                    __del = __getDelegate(false);
                    return __del.ice_isA(__id, __context, __observer);
                }
                catch(IceInternal.LocalExceptionWrapper __ex)
                {
                    __cnt = __handleExceptionWrapperRelaxed(__del, __ex, null, __cnt, __observer);
                }
                catch(LocalException __ex)
                {
                    __cnt = __handleException(__del, __ex, null, __cnt, __observer);
                }
            }
        }
        finally
        {
            if(__observer != null)
            {
                __observer.detach();
            }
        }
    }

    /**
     * Tests whether this proxy supports a given interface.
     *
     * @param __id The Slice type ID of an interface.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_isA(String __id)
    {
        return begin_ice_isA(__id, null, false, null);
    }

    /**
     * Tests whether this proxy supports a given interface.
     *
     * @param __id The Slice type ID of an interface.
     * @param __context The <code>Context</code> map for the invocation.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_isA(String __id, java.util.Map<String, String> __context)
    {
        return begin_ice_isA(__id, __context, true, null);
    }

    /**
     * Tests whether this proxy supports a given interface.
     *
     * @param __id The Slice type ID of an interface.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_isA(String __id, Callback __cb)
    {
        return begin_ice_isA(__id, null, false, __cb);
    }

    /**
     * Tests whether this proxy supports a given interface.
     *
     * @param __id The Slice type ID of an interface.
     * @param __context The <code>Context</code> map for the invocation.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_isA(String __id, java.util.Map<String, String> __context, Callback __cb)
    {
        return begin_ice_isA(__id, __context, true, __cb);
    }

    /**
     * Tests whether this proxy supports a given interface.
     *
     * @param __id The Slice type ID of an interface.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_isA(String __id, Callback_Object_ice_isA __cb)
    {
        return begin_ice_isA(__id, null, false, __cb);
    }

    /**
     * Tests whether this proxy supports a given interface.
     *
     * @param __id The Slice type ID of an interface.
     * @param __context The <code>Context</code> map for the invocation.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_isA(String __id, java.util.Map<String, String> __context, Callback_Object_ice_isA __cb)
    {
        return begin_ice_isA(__id, __context, true, __cb);
    }

    private AsyncResult
    begin_ice_isA(String __id, java.util.Map<String, String> __context, boolean __explicitCtx,
                  IceInternal.CallbackBase __cb)
    {
        IceInternal.OutgoingAsync __result = new IceInternal.OutgoingAsync(this, __ice_isA_name, __cb);
        __checkAsyncTwowayOnly(__ice_isA_name);
        try
        {
            __result.__prepare(__ice_isA_name, OperationMode.Nonmutating, __context, __explicitCtx);
            IceInternal.BasicStream __os = __result.__startWriteParams(Ice.FormatType.DefaultFormat);
            __os.writeString(__id);
            __result.__endWriteParams();
            __result.__send(true);
        }
        catch(LocalException __ex)
        {
            __result.__exceptionAsync(__ex);
        }
        return __result;
    }

    /**
     * Completes the asynchronous ice_isA request.
     *
     * @param __result The asynchronous result.
     * @return <code>true</code> if this proxy supports the specified interface; <code>false</code>, otherwise.
     **/
    public final boolean
    end_ice_isA(AsyncResult __result)
    {
        AsyncResult.__check(__result, this, __ice_isA_name);
        boolean __ok = __result.__wait();
        try
        {
            if(!__ok)
            {
                try
                {
                    __result.__throwUserException();
                }
                catch(UserException __ex)
                {
                    throw new UnknownUserException(__ex.ice_name(), __ex);
                }
            }
            boolean __ret;
            IceInternal.BasicStream __is = __result.__startReadParams();
            __ret = __is.readBool();
            __result.__endReadParams();
            return __ret;
        }
        catch(Ice.LocalException ex)
        {
            InvocationObserver obsv = __result.__getObserver();
            if(obsv != null)
            {
                obsv.failed(ex.ice_name());
            }
            throw ex;
        }
    }

    /**
     * Tests whether the target object of this proxy can be reached.
     **/
    public final void
    ice_ping()
    {
        ice_ping(null, false);
    }

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param __context The <code>Context</code> map for the invocation.
     **/
    public final void
    ice_ping(java.util.Map<String, String> __context)
    {
        ice_ping(__context, true);
    }

    private static final String __ice_ping_name = "ice_ping";

    private void
    ice_ping(java.util.Map<String, String> __context, boolean __explicitCtx)
    {
        if(__explicitCtx && __context == null)
        {
            __context = _emptyContext;
        }

        final InvocationObserver __observer = IceInternal.ObserverHelper.get(this, __ice_ping_name, __context);
        int __cnt = 0;
        try
        {
            while(true)
            {
                _ObjectDel __del = null;
                try
                {
                    __del = __getDelegate(false);
                    __del.ice_ping(__context, __observer);
                    return;
                }
                catch(IceInternal.LocalExceptionWrapper __ex)
                {
                    __cnt = __handleExceptionWrapperRelaxed(__del, __ex, null, __cnt, __observer);
                }
                catch(LocalException __ex)
                {
                    __cnt = __handleException(__del, __ex, null, __cnt, __observer);
                }
            }
        }
        finally
        {
            if(__observer != null)
            {
                __observer.detach();
            }
        }
    }

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_ping()
    {
        return begin_ice_ping(null, false, null);
    }

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param __context The context map for the invocation.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_ping(java.util.Map<String, String> __context)
    {
        return begin_ice_ping(__context, true, null);
    }

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_ping(Callback __cb)
    {
        return begin_ice_ping(null, false, __cb);
    }

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param __context The context map for the invocation.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_ping(java.util.Map<String, String> __context, Callback __cb)
    {
        return begin_ice_ping(__context, true, __cb);
    }

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_ping(Callback_Object_ice_ping __cb)
    {
        return begin_ice_ping(null, false, __cb);
    }

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param __context The context map for the invocation.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_ping(java.util.Map<String, String> __context, Callback_Object_ice_ping __cb)
    {
        return begin_ice_ping(__context, true, __cb);
    }

    private AsyncResult
    begin_ice_ping(java.util.Map<String, String> __context, boolean __explicitCtx, IceInternal.CallbackBase __cb)
    {
        IceInternal.OutgoingAsync __result = new IceInternal.OutgoingAsync(this, __ice_ping_name, __cb);
        try
        {
            __result.__prepare(__ice_ping_name, OperationMode.Nonmutating, __context, __explicitCtx);
            __result.__writeEmptyParams();
            __result.__send(true);
        }
        catch(LocalException __ex)
        {
            __result.__exceptionAsync(__ex);
        }
        return __result;
    }

    /**
     * Completes the asynchronous ping request.
     *
     * @param __result The asynchronous result.
     **/
    public final void
    end_ice_ping(AsyncResult __result)
    {
        __end(__result, __ice_ping_name);
    }

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @return The Slice type IDs of the interfaces supported by the target object, in base-to-derived
     * order. The first element of the returned array is always <code>::Ice::Object</code>.
     **/
    public final String[]
    ice_ids()
    {
        return ice_ids(null, false);
    }

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @param __context The <code>Context</code> map for the invocation.
     * @return The Slice type IDs of the interfaces supported by the target object, in base-to-derived
     * order. The first element of the returned array is always <code>::Ice::Object</code>.
     **/
    public final String[]
    ice_ids(java.util.Map<String, String> __context)
    {
        return ice_ids(__context, true);
    }

    private static final String __ice_ids_name = "ice_ids";

    private String[]
    ice_ids(java.util.Map<String, String> __context, boolean __explicitCtx)
    {
        if(__explicitCtx && __context == null)
        {
            __context = _emptyContext;
        }

        final InvocationObserver __observer = IceInternal.ObserverHelper.get(this, __ice_ids_name, __context);
        int __cnt = 0;
        try
        {
            while(true)
            {
                _ObjectDel __del = null;
                try
                {
                    __checkTwowayOnly(__ice_ids_name);
                    __del = __getDelegate(false);
                    return __del.ice_ids(__context, __observer);
                }
                catch(IceInternal.LocalExceptionWrapper __ex)
                {
                    __cnt = __handleExceptionWrapperRelaxed(__del, __ex, null, __cnt, __observer);
                }
                catch(LocalException __ex)
                {
                    __cnt = __handleException(__del, __ex, null, __cnt, __observer);
                }
            }
        }
        finally
        {
            if(__observer != null)
            {
                __observer.detach();
            }
        }
    }

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_ids()
    {
        return begin_ice_ids(null, false, null);
    }

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @param __context The context map for the invocation.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_ids(java.util.Map<String, String> __context)
    {
        return begin_ice_ids(__context, true, null);
    }

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_ids(Callback __cb)
    {
        return begin_ice_ids(null, false, __cb);
    }

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @param __context The context map for the invocation.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_ids(java.util.Map<String, String> __context, Callback __cb)
    {
        return begin_ice_ids(__context, true, __cb);
    }

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_ids(Callback_Object_ice_ids __cb)
    {
        return begin_ice_ids(null, false, __cb);
    }

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @param __context The context map for the invocation.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_ids(java.util.Map<String, String> __context, Callback_Object_ice_ids __cb)
    {
        return begin_ice_ids(__context, true, __cb);
    }

    private AsyncResult
    begin_ice_ids(java.util.Map<String, String> __context, boolean __explicitCtx, IceInternal.CallbackBase __cb)
    {
        IceInternal.OutgoingAsync __result = new IceInternal.OutgoingAsync(this, __ice_ids_name, __cb);
        __checkAsyncTwowayOnly(__ice_ids_name);
        try
        {
            __result.__prepare(__ice_ids_name, OperationMode.Nonmutating, __context, __explicitCtx);
            __result.__writeEmptyParams();
            __result.__send(true);
        }
        catch(LocalException __ex)
        {
            __result.__exceptionAsync(__ex);
        }
        return __result;
    }

    /**
     * Completes the asynchronous ice_ids request.
     *
     * @param __result The asynchronous result.
     * @return The Slice type IDs of the interfaces supported by the target object, in base-to-derived
     * order. The first element of the returned array is always <code>::Ice::Object</code>.
     **/
    public final String[]
    end_ice_ids(AsyncResult __result)
    {
        AsyncResult.__check(__result, this, __ice_ids_name);
        boolean __ok = __result.__wait();
        try
        {
            if(!__ok)
            {
                try
                {
                    __result.__throwUserException();
                }
                catch(UserException __ex)
                {
                    throw new UnknownUserException(__ex.ice_name(), __ex);
                }
            }
            String[] __ret = null;
            IceInternal.BasicStream __is = __result.__startReadParams();
            __ret = StringSeqHelper.read(__is);
            __result.__endReadParams();
            return __ret;
        }
        catch(Ice.LocalException ex)
        {
            InvocationObserver obsv = __result.__getObserver();
            if(obsv != null)
            {
                obsv.failed(ex.ice_name());
            }
            throw ex;
        }
    }

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @return The Slice type ID of the most-derived interface.
     **/
    public final String
    ice_id()
    {
        return ice_id(null, false);
    }

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @param __context The <code>Context</code> map for the invocation.
     * @return The Slice type ID of the most-derived interface.
     **/
    public final String
    ice_id(java.util.Map<String, String> __context)
    {
        return ice_id(__context, true);
    }

    private static final String __ice_id_name = "ice_id";

    private String
    ice_id(java.util.Map<String, String> __context, boolean __explicitCtx)
    {
        if(__explicitCtx && __context == null)
        {
            __context = _emptyContext;
        }

        final InvocationObserver __observer = IceInternal.ObserverHelper.get(this, __ice_id_name, __context);
        int __cnt = 0;
        try
        {
            while(true)
            {
                _ObjectDel __del = null;
                try
                {
                    __checkTwowayOnly(__ice_id_name);
                    __del = __getDelegate(false);
                    return __del.ice_id(__context, __observer);
                }
                catch(IceInternal.LocalExceptionWrapper __ex)
                {
                    __cnt = __handleExceptionWrapperRelaxed(__del, __ex, null, __cnt, __observer);
                }
                catch(LocalException __ex)
                {
                    __cnt = __handleException(__del, __ex, null, __cnt, __observer);
                }
            }
        }
        finally
        {
            if(__observer != null)
            {
                __observer.detach();
            }
        }
    }

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_id()
    {
        return begin_ice_id(null, false, null);
    }

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @param __context The context map for the invocation.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_id(java.util.Map<String, String> __context)
    {
        return begin_ice_id(__context, true, null);
    }

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_id(Callback __cb)
    {
        return begin_ice_id(null, false, __cb);
    }

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @param __context The context map for the invocation.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_id(java.util.Map<String, String> __context, Callback __cb)
    {
        return begin_ice_id(__context, true, __cb);
    }

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_id(Callback_Object_ice_id __cb)
    {
        return begin_ice_id(null, false, __cb);
    }

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @param __context The context map for the invocation.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    public final AsyncResult
    begin_ice_id(java.util.Map<String, String> __context, Callback_Object_ice_id __cb)
    {
        return begin_ice_id(__context, true, __cb);
    }

    private AsyncResult
    begin_ice_id(java.util.Map<String, String> __context, boolean __explicitCtx, IceInternal.CallbackBase __cb)
    {
        IceInternal.OutgoingAsync __result = new IceInternal.OutgoingAsync(this, __ice_id_name, __cb);
        __checkAsyncTwowayOnly(__ice_id_name);
        try
        {
            __result.__prepare(__ice_id_name, OperationMode.Nonmutating, __context, __explicitCtx);
            __result.__writeEmptyParams();
            __result.__send(true);
        }
        catch(LocalException __ex)
        {
            __result.__exceptionAsync(__ex);
        }
        return __result;
    }

    /**
     * Completes the asynchronous ice_id request.
     *
     * @param __result The asynchronous result.
     * @return The Slice type ID of the most-derived interface.
     **/
    public final String
    end_ice_id(AsyncResult __result)
    {
        AsyncResult.__check(__result, this, __ice_id_name);
        boolean __ok = __result.__wait();
        try
        {
            if(!__ok)
            {
                try
                {
                    __result.__throwUserException();
                }
                catch(UserException __ex)
                {
                    throw new UnknownUserException(__ex.ice_name(), __ex);
                }
            }
            String __ret = null;
            IceInternal.BasicStream __is = __result.__startReadParams();
            __ret = __is.readString();
            __result.__endReadParams();
            return __ret;
        }
        catch(Ice.LocalException ex)
        {
            InvocationObserver obsv = __result.__getObserver();
            if(obsv != null)
            {
                obsv.failed(ex.ice_name());
            }
            throw ex;
        }
    }

    /**
     * Invoke an operation dynamically.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * @param outParams The encoded out-paramaters and return value
     * for the operation. The return value follows any out-parameters.
     * @return If the operation completed successfully, the return value
     * is <code>true</code>. If the operation raises a user exception,
     * the return value is <code>false</code>; in this case, <code>outParams</code>
     * contains the encoded user exception. If the operation raised an
     * it throws it directly.
     *
     * @see Blobject
     * @see OperationMode
     **/
    public final boolean
    ice_invoke(String operation, OperationMode mode, byte[] inParams, ByteSeqHolder outParams)
    {
        return ice_invoke(operation, mode, inParams, outParams, null, false);
    }

    /**
     * Invoke an operation dynamically.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * @param outParams The encoded out-paramaters and return value
     * for the operation. The return value follows any out-parameters.
     * @param __context The context map for the invocation.
     * @return If the operation was invoked synchronously (because there
     * was no need to queue the request), the return value is <code>true</code>;
     * otherwise, if the invocation was queued, the return value is <code>false</code>.
     *
     * @see Blobject
     * @see OperationMode
     **/
    public final boolean
    ice_invoke(String operation, OperationMode mode, byte[] inParams, ByteSeqHolder outParams,
               java.util.Map<String, String> context)
    {
        return ice_invoke(operation, mode, inParams, outParams, context, true);
    }

    private boolean
    ice_invoke(String operation, OperationMode mode, byte[] inParams, ByteSeqHolder outParams,
               java.util.Map<String, String> context, boolean explicitCtx)
    {
        if(explicitCtx && context == null)
        {
            context = _emptyContext;
        }

        final InvocationObserver __observer = IceInternal.ObserverHelper.get(this, operation, context);
        int __cnt = 0;
        try
        {
            while(true)
            {
                _ObjectDel __del = null;
                try
                {
                    __del = __getDelegate(false);
                    return __del.ice_invoke(operation, mode, inParams, outParams, context, __observer);
                }
                catch(IceInternal.LocalExceptionWrapper __ex)
                {
                    if(mode == OperationMode.Nonmutating || mode == OperationMode.Idempotent)
                    {
                        __cnt = __handleExceptionWrapperRelaxed(__del, __ex, null, __cnt, __observer);
                    }
                    else
                    {
                        __handleExceptionWrapper(__del, __ex, __observer);
                    }
                }
                catch(LocalException __ex)
                {
                    __cnt = __handleException(__del, __ex, null, __cnt, __observer);
                }
            }
        }
        finally
        {
            if(__observer != null)
            {
                __observer.detach();
            }
        }
    }

    private static final String __ice_invoke_name = "ice_invoke";

    /**
     * Invokes an operation dynamically and asynchronously.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * @return The asynchronous result object.
     *
     * @see Blobject
     * @see OperationMode
     **/
    public final AsyncResult
    begin_ice_invoke(String operation, OperationMode mode, byte[] inParams)
    {
        return begin_ice_invoke(operation, mode, inParams, null, false, null);
    }

    /**
     * Invokes an operation dynamically and asynchronously.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * for the operation. The return value follows any out-parameters.
     * @param __context The context map for the invocation.
     * @return The asynchronous result object.
     *
     * @see Blobject
     * @see OperationMode
     **/
    public final AsyncResult
    begin_ice_invoke(String operation, OperationMode mode, byte[] inParams,
                                 java.util.Map<String, String> __context)
    {
        return begin_ice_invoke(operation, mode, inParams, __context, true, null);
    }

    /**
     * Invokes an operation dynamically and asynchronously.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * for the operation. The return value follows any out-parameters.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     *
     * @see Blobject
     * @see OperationMode
     **/
    public final AsyncResult
    begin_ice_invoke(String operation, OperationMode mode, byte[] inParams, Callback __cb)
    {
        return begin_ice_invoke(operation, mode, inParams, null, false, __cb);
    }

    /**
     * Invokes an operation dynamically and asynchronously.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * for the operation. The return value follows any out-parameters.
     * @param __context The context map for the invocation.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     *
     * @see Blobject
     * @see OperationMode
     **/
    public final AsyncResult
    begin_ice_invoke(String operation, OperationMode mode, byte[] inParams, java.util.Map<String, String> __context,
                     Callback __cb)
    {
        return begin_ice_invoke(operation, mode, inParams, __context, true, __cb);
    }

    /**
     * Invokes an operation dynamically and asynchronously.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * for the operation. The return value follows any out-parameters.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     *
     * @see Blobject
     * @see OperationMode
     **/
    public final AsyncResult
    begin_ice_invoke(String operation, OperationMode mode, byte[] inParams, Callback_Object_ice_invoke __cb)
    {
        return begin_ice_invoke(operation, mode, inParams, null, false, __cb);
    }

    /**
     * Invokes an operation dynamically and asynchronously.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * for the operation. The return value follows any out-parameters.
     * @param __context The context map for the invocation.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     *
     * @see Blobject
     * @see OperationMode
     **/
    public final AsyncResult
    begin_ice_invoke(String operation, OperationMode mode, byte[] inParams, java.util.Map<String, String> __context,
                     Callback_Object_ice_invoke __cb)
    {
        return begin_ice_invoke(operation, mode, inParams, __context, true, __cb);
    }

    private AsyncResult
    begin_ice_invoke(String operation, OperationMode mode, byte[] inParams, java.util.Map<String, String> __context,
                     boolean __explicitCtx, IceInternal.CallbackBase __cb)
    {
        IceInternal.OutgoingAsync __result = new IceInternal.OutgoingAsync(this, __ice_invoke_name, __cb);
        try
        {
            __result.__prepare(operation, mode, __context, __explicitCtx);
            __result.__writeParamEncaps(inParams);
            __result.__send(true);
        }
        catch(LocalException __ex)
        {
            __result.__exceptionAsync(__ex);
        }
        return __result;
    }

    /**
     * Completes the asynchronous ice_invoke request.
     *
     * @param outParams The encoded out-paramaters and return value.
     * @param __result The asynchronous result.
     * @return If the operation completed successfully, the return value
     * is <code>true</code>. If the operation raises a user exception,
     * the return value is <code>false</code>; in this case, <code>outParams</code>
     * contains the encoded user exception. If the operation raises a run-time exception,
     * it throws it directly.
     **/
    public final boolean
    end_ice_invoke(ByteSeqHolder outParams, AsyncResult __result)
    {
        AsyncResult.__check(__result, this, __ice_invoke_name);
        boolean ok = __result.__wait();
        if(_reference.getMode() == IceInternal.Reference.ModeTwoway)
        {
            try
            {
                if(outParams != null)
                {
                    outParams.value = __result.__readParamEncaps();
                }
            }
            catch(Ice.LocalException ex)
            {
                InvocationObserver obsv = __result.__getObserver();
                if(obsv != null)
                {
                    obsv.failed(ex.ice_name());
                }
                throw ex;
            }
        }
        return ok;
    }

    /**
     * Invokes an operation dynamically and asynchronously.
     *
     * @param cb The callback object to notify when the operation completes.
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * @return If the operation was invoked synchronously (because there
     * was no need to queue the request), the return value is <code>true</code>;
     * otherwise, if the invocation was queued, the return value is <code>false</code>.
     *
     * @see AMI_Object_ice_invoke
     * @see OperationMode
     **/
    public final boolean
    ice_invoke_async(AMI_Object_ice_invoke cb, String operation, OperationMode mode, byte[] inParams)
    {
        AsyncResult __result = begin_ice_invoke(operation, mode, inParams, cb);
        return __result.sentSynchronously();
    }

    /**
     * Invokes an operation dynamically and asynchronously.
     *
     * @param cb The callback object to notify when the operation completes.
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * @param __context The context map for the invocation.
     * @return If the operation was invoked synchronously (because there
     * was no need to queue the request), the return value is <code>true</code>;
     * otherwise, if the invocation was queued, the return value is <code>false</code>.
     *
     * @see AMI_Object_ice_invoke
     * @see OperationMode
     **/
    public final boolean
    ice_invoke_async(AMI_Object_ice_invoke cb, String operation, OperationMode mode, byte[] inParams,
                     java.util.Map<String, String> context)
    {
        AsyncResult __result = begin_ice_invoke(operation, mode, inParams, context, cb);
        return __result.sentSynchronously();
    }

    /**
     * Returns the identity embedded in this proxy.
     *
     * @return The identity of the target object.
     **/
    public final Identity
    ice_getIdentity()
    {
        return (Identity)_reference.getIdentity().clone();
    }

    /**
     * Creates a new proxy that is identical to this proxy, except for the identity.
     *
     * @param newIdentity The identity for the new proxy.
     * @return The proxy with the new identity.
     **/
    public final ObjectPrx
    ice_identity(Identity newIdentity)
    {
        if(newIdentity.name.equals(""))
        {
            throw new IllegalIdentityException();
        }
        if(newIdentity.equals(_reference.getIdentity()))
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeIdentity(newIdentity));
            return proxy;
        }
    }

    /**
     * Returns the per-proxy context for this proxy.
     *
     * @return The per-proxy context. If the proxy does not have a per-proxy (implicit) context, the return value
     * is <code>null</code>.
     **/
    public final java.util.Map<String, String>
    ice_getContext()
    {
        return new java.util.HashMap<String, String>(_reference.getContext());
    }

    /**
     * Creates a new proxy that is identical to this proxy, except for the per-proxy context.
     *
     * @param newContext The context for the new proxy.
     * @return The proxy with the new per-proxy context.
     **/
    public final ObjectPrx
    ice_context(java.util.Map<String, String> newContext)
    {
        return newInstance(_reference.changeContext(newContext));
    }

    /**
     * Returns the facet for this proxy.
     *
     * @return The facet for this proxy. If the proxy uses the default facet, the return value is the empty string.
     **/
    public final String
    ice_getFacet()
    {
        return _reference.getFacet();
    }

    /**
     * Creates a new proxy that is identical to this proxy, except for the facet.
     *
     * @param newFacet The facet for the new proxy.
     * @return The proxy with the new facet.
     **/
    public final ObjectPrx
    ice_facet(String newFacet)
    {
        if(newFacet == null)
        {
            newFacet = "";
        }

        if(newFacet.equals(_reference.getFacet()))
        {
            return this;
        }
        else
        {
            ObjectPrxHelperBase proxy = new ObjectPrxHelperBase();
            proxy.setup(_reference.changeFacet(newFacet));
            return proxy;
        }
    }

    /**
     * Returns the adapter ID for this proxy.
     *
     * @return The adapter ID. If the proxy does not have an adapter ID, the return value is the empty string.
     **/
    public final String
    ice_getAdapterId()
    {
        return _reference.getAdapterId();
    }

    /**
     * Creates a new proxy that is identical to this proxy, except for the adapter ID.
     *
     * @param newAdapterId The adapter ID for the new proxy.
     * @return The proxy with the new adapter ID.
     **/
    public final ObjectPrx
    ice_adapterId(String newAdapterId)
    {
        if(newAdapterId == null)
        {
            newAdapterId = "";
        }

        if(newAdapterId.equals(_reference.getAdapterId()))
        {
            return this;
        }
        else
        {
            return newInstance(_reference.changeAdapterId(newAdapterId));
        }
    }

    /**
     * Returns the endpoints used by this proxy.
     *
     * @return The endpoints used by this proxy.
     *
     * @see Endpoint
     **/
    public final Endpoint[]
    ice_getEndpoints()
    {
        return _reference.getEndpoints().clone();
    }

    /**
     * Creates a new proxy that is identical to this proxy, except for the endpoints.
     *
     * @param newEndpoints The endpoints for the new proxy.
     * @return The proxy with the new endpoints.
     **/
    public final ObjectPrx
    ice_endpoints(Endpoint[] newEndpoints)
    {
        if(java.util.Arrays.equals(newEndpoints, _reference.getEndpoints()))
        {
            return this;
        }
        else
        {
            IceInternal.EndpointI[] edpts = new IceInternal.EndpointI[newEndpoints.length];
            edpts = (IceInternal.EndpointI[])java.util.Arrays.asList(newEndpoints).toArray(edpts);
            return newInstance(_reference.changeEndpoints(edpts));
        }
    }

    /**
     * Returns the locator cache timeout of this proxy.
     *
     * @return The locator cache timeout value (in seconds).
     *
     * @see Locator
     **/
    public final int
    ice_getLocatorCacheTimeout()
    {
        return _reference.getLocatorCacheTimeout();
    }

    /**
     * Returns the connection id of this proxy.
     *
     * @return The connection id.
     *
     **/
    public final String
    ice_getConnectionId()
    {
        return _reference.getConnectionId();
    }

    /**
     * Creates a new proxy that is identical to this proxy, except for the locator cache timeout.
     *
     * @param newTimeout The new locator cache timeout (in seconds).
     *
     * @see Locator
     **/
    public final ObjectPrx
    ice_locatorCacheTimeout(int newTimeout)
    {
        if(newTimeout == _reference.getLocatorCacheTimeout())
        {
            return this;
        }
        else
        {
            return newInstance(_reference.changeLocatorCacheTimeout(newTimeout));
        }
    }

    /**
     * Returns whether this proxy caches connections.
     *
     * @return <code>true</code> if this proxy caches connections; <code>false</code>, otherwise.
     **/
    public final boolean
    ice_isConnectionCached()
    {
        return _reference.getCacheConnection();
    }

    /**
     * Creates a new proxy that is identical to this proxy, except for connection caching.
     *
     * @param newCache <code>true</code> if the new proxy should cache connections; <code>false</code>, otherwise.
     * @return The new proxy with the specified caching policy.
     **/
    public final ObjectPrx
    ice_connectionCached(boolean newCache)
    {
        if(newCache == _reference.getCacheConnection())
        {
            return this;
        }
        else
        {
            return newInstance(_reference.changeCacheConnection(newCache));
        }
    }

    /**
     * Returns how this proxy selects endpoints (randomly or ordered).
     *
     * @return The endpoint selection policy.
     *
     * @see EndpointSelectionType
     **/
    public final Ice.EndpointSelectionType
    ice_getEndpointSelection()
    {
        return _reference.getEndpointSelection();
    }

    /**
     * Creates a new proxy that is identical to this proxy, except for the endpoint selection policy.
     *
     * @param newType The new endpoint selection policy.
     * @return The new proxy with the specified endpoint selection policy.
     *
     * @see EndpointSelectionType
     **/
    public final ObjectPrx
    ice_endpointSelection(Ice.EndpointSelectionType newType)
    {
        if(newType == _reference.getEndpointSelection())
        {
            return this;
        }
        else
        {
            return newInstance(_reference.changeEndpointSelection(newType));
        }
    }

    /**
     * Returns whether this proxy uses only secure endpoints.
     *
     * @return <code>true</code> if all endpoints for this proxy are secure; <code>false</code>, otherwise.
     **/
    public final boolean
    ice_isSecure()
    {
        return _reference.getSecure();
    }

    /**
     * Creates a new proxy that is identical to this proxy, except for its endpoints.
     *
     * @param If <code>b</code> is <code>true</code>, only endpoints that use a secure transport are
     * retained for the new proxy. If <code>b</code> is false, the returned proxy is identical to this proxy.
     * @return The new proxy with possible different endpoints.k
     **/
    public final ObjectPrx
    ice_secure(boolean b)
    {
        if(b == _reference.getSecure())
        {
            return this;
        }
        else
        {
            return newInstance(_reference.changeSecure(b));
        }
    }

    /**
     * Creates a new proxy that is identical to this proxy, except for the encoding used to marshal
     * parameters.
     *
     * @param e The encoding version to use to marshal requests parameters.
     * @return The new proxy with the specified encoding version.
     **/
    public final ObjectPrx
    ice_encodingVersion(Ice.EncodingVersion e)
    {
        if(e.equals(_reference.getEncoding()))
        {
            return this;
        }
        else
        {
            return newInstance(_reference.changeEncoding(e));
        }
    }

    /**
     * Returns the encoding version used to marshal requests parameters.
     *
     * @return The encoding version.
     **/
    public final Ice.EncodingVersion 
    ice_getEncodingVersion()
    {
        return (Ice.EncodingVersion)_reference.getEncoding().clone();
    }

    /**
     * Returns whether this proxy prefers secure endpoints.
     *
     * @return <code>true</code> if the proxy always attempts to invoke via secure endpoints before it
     * attempts to use insecure endpoints; <code>false</code>, otherwise;
     **/
    public final boolean
    ice_isPreferSecure()
    {
        return _reference.getPreferSecure();
    }

    /**
     * Creates a new proxy that is identical to this proxy, except for its endpoint selection policy.
     *
     * @param b If <code>b</code> is <code>true</code>, the new proxy will use secure endpoints for invocations
     * and only use insecure endpoints if an invocation cannot be made via secure endpoints. If <code>b</code> is
     * <code>false</code>, the proxy prefers insecure endpoints to secure ones.
     * @return The new proxy with the new endpoint selection policy.
     **/
    public final ObjectPrx
    ice_preferSecure(boolean b)
    {
        if(b == _reference.getPreferSecure())
        {
            return this;
        }
        else
        {
            return newInstance(_reference.changePreferSecure(b));
        }
    }

    /**
     * Returns the router for this proxy.
     *
     * @return The router for the proxy. If no router is configured for the proxy, the return value
     * is <code>null</code>.
     **/
    public final Ice.RouterPrx
    ice_getRouter()
    {
        IceInternal.RouterInfo ri = _reference.getRouterInfo();
        return ri != null ? ri.getRouter() : null;
    }

    /**
     * Creates a new proxy that is identical to this proxy, except for the router.
     *
     * @param router The router for the new proxy.
     * @return The new proxy with the specified router.
     **/
    public final ObjectPrx
    ice_router(Ice.RouterPrx router)
    {
        IceInternal.Reference ref = _reference.changeRouter(router);
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            return newInstance(ref);
        }
    }

    /**
     * Returns the locator for this proxy.
     *
     * @return The locator for this proxy. If no locator is configured, the return value is <code>null</code>.
     **/
    public final Ice.LocatorPrx
    ice_getLocator()
    {
        IceInternal.LocatorInfo ri = _reference.getLocatorInfo();
        return ri != null ? ri.getLocator() : null;
    }

    /**
     * Creates a new proxy that is identical to this proxy, except for the locator.
     *
     * @param The locator for the new proxy.
     * @return The new proxy with the specified locator.
     **/
    public final ObjectPrx
    ice_locator(Ice.LocatorPrx locator)
    {
        IceInternal.Reference ref = _reference.changeLocator(locator);
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            return newInstance(ref);
        }
    }

    /**
     * Returns whether this proxy uses collocation optimization.
     *
     * @return <code>true</code> if the proxy uses collocation optimization; <code>false</code>, otherwise.
     **/
    public final boolean
    ice_isCollocationOptimized()
    {
        return _reference.getCollocationOptimized();
    }

    /**
     * Creates a new proxy that is identical to this proxy, except for collocation optimization.
     *
     * @param b <code>true</code> if the new proxy enables collocation optimization; <code>false</code>, otherwise.
     * @return The new proxy the specified collocation optimization.
     **/
    public final ObjectPrx
    ice_collocationOptimized(boolean b)
    {
        if(b == _reference.getCollocationOptimized())
        {
            return this;
        }
        else
        {
            return newInstance(_reference.changeCollocationOptimized(b));
        }
    }

    /**
     * Creates a new proxy that is identical to this proxy, but uses twoway invocations.
     *
     * @return A new proxy that uses twoway invocations.
     **/
    public final ObjectPrx
    ice_twoway()
    {
        if(_reference.getMode() == IceInternal.Reference.ModeTwoway)
        {
            return this;
        }
        else
        {
            return newInstance(_reference.changeMode(IceInternal.Reference.ModeTwoway));
        }
    }

    /**
     * Returns whether this proxy uses twoway invocations.
     * @return <code>true</code> if this proxy uses twoway invocations; <code>false</code>, otherwise.
     **/
    public final boolean
    ice_isTwoway()
    {
        return _reference.getMode() == IceInternal.Reference.ModeTwoway;
    }

    /**
     * Creates a new proxy that is identical to this proxy, but uses oneway invocations.
     *
     * @return A new proxy that uses oneway invocations.
     **/
    public final ObjectPrx
    ice_oneway()
    {
        if(_reference.getMode() == IceInternal.Reference.ModeOneway)
        {
            return this;
        }
        else
        {
            return newInstance(_reference.changeMode(IceInternal.Reference.ModeOneway));
        }
    }

    /**
     * Returns whether this proxy uses oneway invocations.
     * @return <code>true</code> if this proxy uses oneway invocations; <code>false</code>, otherwise.
     **/
    public final boolean
    ice_isOneway()
    {
        return _reference.getMode() == IceInternal.Reference.ModeOneway;
    }

    /**
     * Creates a new proxy that is identical to this proxy, but uses batch oneway invocations.
     *
     * @return A new proxy that uses batch oneway invocations.
     **/
    public final ObjectPrx
    ice_batchOneway()
    {
        if(_reference.getMode() == IceInternal.Reference.ModeBatchOneway)
        {
            return this;
        }
        else
        {
            return newInstance(_reference.changeMode(IceInternal.Reference.ModeBatchOneway));
        }
    }

    /**
     * Returns whether this proxy uses batch oneway invocations.
     * @return <code>true</code> if this proxy uses batch oneway invocations; <code>false</code>, otherwise.
     **/
    public final boolean
    ice_isBatchOneway()
    {
        return _reference.getMode() == IceInternal.Reference.ModeBatchOneway;
    }

    /**
     * Creates a new proxy that is identical to this proxy, but uses datagram invocations.
     *
     * @return A new proxy that uses datagram invocations.
     **/
    public final ObjectPrx
    ice_datagram()
    {
        if(_reference.getMode() == IceInternal.Reference.ModeDatagram)
        {
            return this;
        }
        else
        {
            return newInstance(_reference.changeMode(IceInternal.Reference.ModeDatagram));
        }
    }

    /**
     * Returns whether this proxy uses datagram invocations.
     * @return <code>true</code> if this proxy uses datagram invocations; <code>false</code>, otherwise.
     **/
    public final boolean
    ice_isDatagram()
    {
        return _reference.getMode() == IceInternal.Reference.ModeDatagram;
    }

    /**
     * Creates a new proxy that is identical to this proxy, but uses batch datagram invocations.
     *
     * @return A new proxy that uses batch datagram invocations.
     **/
    public final ObjectPrx
    ice_batchDatagram()
    {
        if(_reference.getMode() == IceInternal.Reference.ModeBatchDatagram)
        {
            return this;
        }
        else
        {
            return newInstance(_reference.changeMode(IceInternal.Reference.ModeBatchDatagram));
        }
    }

    /**
     * Returns whether this proxy uses batch datagram invocations.
     * @return <code>true</code> if this proxy uses batch datagram invocations; <code>false</code>, otherwise.
     **/
    public final boolean
    ice_isBatchDatagram()
    {
        return _reference.getMode() == IceInternal.Reference.ModeBatchDatagram;
    }

    /**
     * Creates a new proxy that is identical to this proxy, except for compression.
     *
     * @param co <code>true</code> enables compression for the new proxy; <code>false</code>disables compression.
     * @return A new proxy with the specified compression setting.
     **/
    public final ObjectPrx
    ice_compress(boolean co)
    {
        IceInternal.Reference ref = _reference.changeCompress(co);
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            return newInstance(ref);
        }
    }

    /**
     * Creates a new proxy that is identical to this proxy, except for its timeout setting.
     *
     * @param t The timeout for the new proxy in milliseconds.
     * @return A new proxy with the specified timeout.
     **/
    public final ObjectPrx
    ice_timeout(int t)
    {
        IceInternal.Reference ref = _reference.changeTimeout(t);
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            return newInstance(ref);
        }
    }

    /**
     * Creates a new proxy that is identical to this proxy, except for its connection ID.
     *
     * @param connectionId The connection ID for the new proxy. An empty string removes the
     * connection ID.
     *
     * @return A new proxy with the specified connection ID.
     **/
    public final ObjectPrx
    ice_connectionId(String id)
    {
        IceInternal.Reference ref = _reference.changeConnectionId(id);
        if(ref.equals(_reference))
        {
            return this;
        }
        else
        {
            return newInstance(ref);
        }
    }

    /**
     * Returns the {@link Connection} for this proxy. If the proxy does not yet have an established connection,
     * it first attempts to create a connection.
     *
     * @return The {@link Connection} for this proxy.
     * @throws CollocationOptimizationException If the proxy uses collocation optimization and denotes a
     * collocated object.
     *
     * @see Connection
     **/
    public final Connection
    ice_getConnection()
    {
        final InvocationObserver __observer = IceInternal.ObserverHelper.get(this, "ice_getConnection");
        int __cnt = 0;
        try
        {
            while(true)
            {
                _ObjectDel __del = null;
                try
                {
                    __del = __getDelegate(false);
                    // Wait for the connection to be established.
                    return __del.__getRequestHandler().getConnection(true);
                    
                }
                catch(LocalException __ex)
                {
                    __cnt = __handleException(__del, __ex, null, __cnt, __observer);
                }
            }
        }
        finally
        {
            if(__observer != null)
            {
                __observer.detach();
            }
        }
    }

    /**
     * Returns the cached {@link Connection} for this proxy. If the proxy does not yet have an established
     * connection, it does not attempt to create a connection.
     *
     * @return The cached {@link Connection} for this proxy (<code>null</code> if the proxy does not have
     * an established connection).
     * @throws CollocationOptimizationException If the proxy uses collocation optimization and denotes a
     * collocated object.
     *
     * @see Connection
     **/
    public final Connection
    ice_getCachedConnection()
    {
        _ObjectDel __del = null;
        synchronized(this)
        {
            __del = _delegate;
        }

        if(__del != null)
        {
            try
            {
                // Don't wait for the connection to be established.
                return __del.__getRequestHandler().getConnection(false);
            }
            catch(LocalException ex)
            {
            }
        }
        return null;
    }

    /**
     * Flushes any pending batched requests for this communicator. The call blocks until the flush is complete.
     **/
    public void
    ice_flushBatchRequests()
    {
        //
        // We don't automatically retry if ice_flushBatchRequests fails. Otherwise, if some batch
        // requests were queued with the connection, they would be lost without being noticed.
        //
        final InvocationObserver __observer = IceInternal.ObserverHelper.get(this, __ice_flushBatchRequests_name);
        _ObjectDel __del = null;
        int __cnt = -1; // Don't retry.
        try
        {
            __del = __getDelegate(false);
            __del.ice_flushBatchRequests(__observer);
            return;
        }
        catch(LocalException __ex)
        {
            __cnt = __handleException(__del, __ex, null, __cnt, __observer);
        }
    }

    /**
     * Asynchronously flushes any pending batched requests for this communicator. The call does not block.
     *
     * @param cb The callback object to notify the application when the flush is complete.
     * @return <code>true</code> if the requests were flushed immediately without blocking; <code>false</code>
     * if the requests could not be flushed immediately.
     **/
    public boolean
    ice_flushBatchRequests_async(AMI_Object_ice_flushBatchRequests cb)
    {
        AsyncResult result = begin_ice_flushBatchRequests(cb);
        return result.sentSynchronously();
    }

    /**
     * Asynchronously flushes any pending batched requests for this communicator. The call does not block.
     *
     * @return The asynchronous result object.
     **/
    public AsyncResult
    begin_ice_flushBatchRequests()
    {
        AsyncResult result = begin_ice_flushBatchRequestsInternal(null);
        return result;
    }

    /**
     * Asynchronously flushes any pending batched requests for this communicator. The call does not block.
     *
     * @param __cb The callback object to notify the application when the flush is complete.
     * @return The asynchronous result object.
     **/
    public AsyncResult
    begin_ice_flushBatchRequests(Callback __cb)
    {
        AsyncResult result = begin_ice_flushBatchRequestsInternal(__cb);
        return result;
    }

    /**
     * Asynchronously flushes any pending batched requests for this communicator. The call does not block.
     *
     * @param __cb The callback object to notify the application when the flush is complete.
     * @return The asynchronous result object.
     **/
    public AsyncResult
    begin_ice_flushBatchRequests(Callback_Object_ice_flushBatchRequests __cb)
    {
        AsyncResult result = begin_ice_flushBatchRequestsInternal(__cb);
        return result;
    }

    private static final String __ice_flushBatchRequests_name = "ice_flushBatchRequests";

    private AsyncResult
    begin_ice_flushBatchRequestsInternal(IceInternal.CallbackBase __cb)
    {
        IceInternal.ProxyBatchOutgoingAsync __result =
            new IceInternal.ProxyBatchOutgoingAsync(this, __ice_flushBatchRequests_name, __cb);
        try
        {
            __result.__send();
        }
        catch(LocalException __ex)
        {
            __result.__exceptionAsync(__ex);
        }
        return __result;
    }

    public void
    end_ice_flushBatchRequests(AsyncResult __result)
    {
        AsyncResult.__check(__result, this, __ice_flushBatchRequests_name);
        __result.__wait();
    }

    /**
     * Returns whether this proxy equals the passed object. Two proxies are equal if they are equal in all respects,
     * that is, if their object identity, endpoints timeout settings, and so on are all equal.
     *
     * @param r The object to compare this proxy with.
     * @return <code>true</code> if this proxy is equal to <code>r</code>; <code>false</code>, otherwise.
     **/
    public final boolean
    equals(java.lang.Object r)
    {
        if(this == r)
        {
            return true;
        }

        if(r instanceof ObjectPrxHelperBase)
        {
            return _reference.equals(((ObjectPrxHelperBase)r)._reference);
        }

        return false;
    }

    public final IceInternal.Reference
    __reference()
    {
        return _reference;
    }

    public final void
    __copyFrom(ObjectPrx from)
    {
        ObjectPrxHelperBase h = (ObjectPrxHelperBase)from;
        IceInternal.Reference ref = null;
        _ObjectDelM delegateM = null;
        _ObjectDelD delegateD = null;

        synchronized(from)
        {
            ref = h._reference;
            try
            {
                delegateM = (_ObjectDelM)h._delegate;
            }
            catch(ClassCastException ex)
            {
            }
            try
            {
                delegateD = (_ObjectDelD)h._delegate;
            }
            catch(ClassCastException ex)
            {
            }
        }

        //
        // No need to synchronize "*this", as this operation is only
        // called upon initialization.
        //

        assert(_reference == null);
        assert(_delegate == null);

        _reference = ref;

        if(_reference.getCacheConnection())
        {
            //
            // The _delegate attribute is only used if "cache connection"
            // is enabled. If it's not enabled, we don't keep track of the
            // delegate -- a new delegate is created for each invocation.
            //

            if(delegateD != null)
            {
                _ObjectDelD delegate = __createDelegateD();
                delegate.__copyFrom(delegateD);
                _delegate = delegate;
            }
            else if(delegateM != null)
            {
                _ObjectDelM delegate = __createDelegateM();
                delegate.__copyFrom(delegateM);
                _delegate = delegate;
            }
        }
    }

    public final int
    __handleException(_ObjectDel delegate, LocalException ex, Ice.IntHolder interval, int cnt, 
                      InvocationObserver obsv)
    {
        //
        // Only _delegate needs to be mutex protected here.
        //
        synchronized(this)
        {
            if(delegate == _delegate)
            {
                _delegate = null;
            }
        }

        try
        {
            if(cnt == -1) // Don't retry if the retry count is -1.
            {
                throw ex;
            }

            try
            {
                cnt = _reference.getInstance().proxyFactory().checkRetryAfterException(ex, _reference, interval, 
                                                                                            cnt);
            }
            catch(CommunicatorDestroyedException e)
            {
                //
                // The communicator is already destroyed, so we cannot
                // retry.
                //
                throw e;
            }
            if(obsv != null)
            {
                obsv.retried();
            }
            return cnt;
        }
        catch(Ice.LocalException e)
        {
            if(obsv != null)
            {
                obsv.failed(e.ice_name());
            }
            throw e;
        }
    }

    public final void
    __handleExceptionWrapper(_ObjectDel delegate, IceInternal.LocalExceptionWrapper ex, InvocationObserver obsv)
    {
        synchronized(this)
        {
            if(delegate == _delegate)
            {
                _delegate = null;
            }
        }

        if(!ex.retry())
        {
            if(obsv != null)
            {
                obsv.failed(ex.get().ice_name());
            }
            throw ex.get();
        }
    }

    public final int
    __handleExceptionWrapperRelaxed(_ObjectDel delegate,
                                    IceInternal.LocalExceptionWrapper ex, 
                                    Ice.IntHolder interval,
                                    int cnt, 
                                    InvocationObserver obsv)
    {
        if(!ex.retry())
        {
            return __handleException(delegate, ex.get(), interval, cnt, obsv);
        }
        else
        {
            synchronized(this)
            {
                if(delegate == _delegate)
                {
                    _delegate = null;
                }
            }
            return cnt;
        }
    }

    public final void
    __checkTwowayOnly(String name)
    {
        //
        // No mutex lock necessary, there is nothing mutable in this
        // operation.
        //

        if(!ice_isTwoway())
        {
            TwowayOnlyException ex = new TwowayOnlyException();
            ex.operation = name;
            throw ex;
        }
    }

    public final void
    __checkAsyncTwowayOnly(String name)
    {
        //
        // No mutex lock necessary, there is nothing mutable in this
        // operation.
        //

        if(!ice_isTwoway())
        {
            throw new java.lang.IllegalArgumentException("`" + name + "' can only be called with a twoway proxy");
        }
    }

    public final _ObjectDel
    __getDelegate(boolean ami)
    {
        if(_reference.getCacheConnection())
        {
            synchronized(this)
            {
                if(_delegate != null)
                {
                    return _delegate;
                }
                // Connect asynchrously to avoid blocking with the proxy mutex locked.
                _delegate = createDelegate(true);
                return _delegate;
            }
        }
        else
        {
            final int mode = _reference.getMode();
            return createDelegate(ami ||
                                  mode == IceInternal.Reference.ModeBatchOneway ||
                                  mode == IceInternal.Reference.ModeBatchDatagram);
        }
    }

    synchronized public void
    __setRequestHandler(_ObjectDel delegate, IceInternal.RequestHandler handler)
    {
        if(_reference.getCacheConnection())
        {
            if(delegate == _delegate)
            {
                if(_delegate instanceof _ObjectDelM)
                {
                    _delegate = __createDelegateM();
                    _delegate.__setRequestHandler(handler);
                }
                else if(_delegate instanceof _ObjectDelD)
                {
                    _delegate = __createDelegateD();
                    _delegate.__setRequestHandler(handler);
                }
            }
        }
    }

    public final void
    __end(AsyncResult __result, String operation)
    {
        AsyncResult.__check(__result, this, operation);
        boolean ok = __result.__wait();
        if(_reference.getMode() == IceInternal.Reference.ModeTwoway)
        {
            try
            {
                if(!ok)
                {
                    try
                    {
                        __result.__throwUserException();
                    }
                    catch(UserException __ex)
                    {
                        throw new UnknownUserException(__ex.ice_name(), __ex);
                    }
                }
                __result.__readEmptyParams();
            }
            catch(Ice.LocalException ex)
            {
                InvocationObserver obsv = __result.__getObserver();
                if(obsv != null)
                {
                    obsv.failed(ex.ice_name());
                }
                throw ex;
            }
        }
    }

    protected _ObjectDelM
    __createDelegateM()
    {
        return new _ObjectDelM();
    }

    protected _ObjectDelD
    __createDelegateD()
    {
        return new _ObjectDelD();
    }

    _ObjectDel
    createDelegate(boolean async)
    {
        if(_reference.getCollocationOptimized())
        {
            ObjectAdapter adapter = _reference.getInstance().objectAdapterFactory().findObjectAdapter(this);
            if(adapter != null)
            {
                _ObjectDelD d = __createDelegateD();
                d.setup(_reference, adapter);
                return d;
            }
        }

        _ObjectDelM d = __createDelegateM();
        d.setup(_reference, this, async);
        return d;
    }

    //
    // Only for use by IceInternal.ProxyFactory
    //
    public final void
    setup(IceInternal.Reference ref)
    {
        //
        // No need to synchronize, as this operation is only called
        // upon initial initialization.
        //

        assert(_reference == null);
        assert(_delegate == null);

        _reference = ref;
    }

    private final ObjectPrxHelperBase
    newInstance(IceInternal.Reference ref)
    {
        try
        {
            ObjectPrxHelperBase proxy = (ObjectPrxHelperBase)getClass().newInstance();
            proxy.setup(ref);
            return proxy;
        }
        catch(InstantiationException e)
        {
            //
            // Impossible
            //
            assert false;
            return null;
        }
        catch(IllegalAccessException e)
        {
            //
            // Impossible
            //
            assert false;
            return null;
        }
    }

    private void
    writeObject(java.io.ObjectOutputStream out)
        throws java.io.IOException
    {
        out.writeUTF(toString());
    }

    private void
    readObject(java.io.ObjectInputStream in)
         throws java.io.IOException, ClassNotFoundException
    {
        String s = in.readUTF();
        try
        {
            Communicator communicator = ((Ice.ObjectInputStream)in).getCommunicator();
            if(communicator == null)
            {
                throw new java.io.IOException("Cannot deserialize proxy: no communicator provided");
            }
            ObjectPrxHelperBase proxy = (ObjectPrxHelperBase)communicator.stringToProxy(s);
            _reference = proxy._reference;
            assert(proxy._delegate == null);
        }
        catch(ClassCastException ex)
        {
            java.io.IOException e =
                new java.io.IOException("Cannot deserialize proxy: Ice.ObjectInputStream not found");
            e.initCause(ex);
            throw e;
        }
        catch(LocalException ex)
        {
            java.io.IOException e = new java.io.IOException("Failure occurred while deserializing proxy");
            e.initCause(ex);
            throw e;
        }
    }

    protected static final java.util.Map<String, String> _emptyContext = new java.util.HashMap<String, String>();

    private transient IceInternal.Reference _reference;
    private transient _ObjectDel _delegate;
    public static final long serialVersionUID = 0L;
}
