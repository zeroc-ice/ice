// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Base interface of all object proxies.
 **/
public interface ObjectPrx
{
    /**
     * Returns the communicator that created this proxy.
     *
     * @return The communicator that created this proxy.
     **/
    Communicator ice_getCommunicator();

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param __id The type ID of the Slice interface to test against.
     * @return <code>true</code> if the target object has the interface
     * specified by <code>__id</code> or derives from the interface
     * specified by <code>__id</code>.
     **/
    boolean ice_isA(String __id);

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param __id The type ID of the Slice interface to test against.
     * @param __context The context map for the invocation.
     * @return <code>true</code> if the target object has the interface
     * specified by <code>__id</code> or derives from the interface
     * specified by <code>__id</code>.
     **/
    boolean ice_isA(String __id, java.util.Map<String, String> __context);

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param __id The type ID of the Slice interface to test against.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_isA(String __id);

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param __id The type ID of the Slice interface to test against.
     * @param __context The context map for the invocation.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_isA(String __id, java.util.Map<String, String> __context);

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param __id The type ID of the Slice interface to test against.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_isA(String __id, Callback __cb);

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param __id The type ID of the Slice interface to test against.
     * @param __context The context map for the invocation.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_isA(String __id, java.util.Map<String, String> __context, Callback __cb);

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param __id The type ID of the Slice interface to test against.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_isA(String __id, Callback_Object_ice_isA __cb);

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param __id The type ID of the Slice interface to test against.
     * @param __context The context map for the invocation.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_isA(String __id, java.util.Map<String, String> __context, Callback_Object_ice_isA __cb);

    /**
     * Tests whether this proxy supports a given interface.
     *
     * @param __id The Slice type ID of an interface.
     * @param __responseCb The asynchronous response callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_isA(String __id, IceInternal.Functional_BoolCallback __responseCb,
                              IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb);

    /**
     * Tests whether this proxy supports a given interface.
     *
     * @param __id The Slice type ID of an interface.
     * @param __responseCb The asynchronous response callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @param __sentCb The asynchronous sent callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_isA(String __id, IceInternal.Functional_BoolCallback __responseCb,
                              IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb,
                              IceInternal.Functional_BoolCallback __sentCb);

    /**
     * Tests whether this proxy supports a given interface.
     *
     * @param __id The Slice type ID of an interface.
     * @param __context The <code>Context</code> map for the invocation.
     * @param __responseCb The asynchronous response callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_isA(String __id, java.util.Map<String, String> __context,
                              IceInternal.Functional_BoolCallback __responseCb,
                              IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb);

    /**
     * Tests whether this proxy supports a given interface.
     *
     * @param __id The Slice type ID of an interface.
     * @param __context The <code>Context</code> map for the invocation.
     * @param __responseCb The asynchronous response callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @param __sentCb The asynchronous sent callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_isA(String __id, java.util.Map<String, String> __context,
                              IceInternal.Functional_BoolCallback __responseCb,
                              IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb,
                              IceInternal.Functional_BoolCallback __sentCb);

    /**
     * Completes the asynchronous ice_isA request.
     *
     * @param __result The asynchronous result.
     * @return <code>true</code> if this proxy supports the specified interface; <code>false</code>, otherwise.
     **/
    boolean end_ice_isA(AsyncResult __result);

    /**
     * Tests whether the target object of this proxy can be reached.
     **/
    void ice_ping();

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param __context The context map for the invocation.
     **/
    void ice_ping(java.util.Map<String, String> __context);

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_ping();

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param __context The context map for the invocation.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_ping(java.util.Map<String, String> __context);

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_ping(Callback __cb);

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param __context The context map for the invocation.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_ping(java.util.Map<String, String> __context, Callback __cb);

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_ping(Callback_Object_ice_ping __cb);

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param __context The context map for the invocation.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_ping(java.util.Map<String, String> __context, Callback_Object_ice_ping __cb);

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param __responseCb The asynchronous response callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_ping(IceInternal.Functional_VoidCallback __responseCb,
                               IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb);

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param __responseCb The asynchronous response callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @param __sentCb The asynchronous sent callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_ping(IceInternal.Functional_VoidCallback __responseCb,
                               IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb,
                               IceInternal.Functional_BoolCallback __sentCb);

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param __context The context map for the invocation.
     * @param __responseCb The asynchronous response callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_ping(java.util.Map<String, String> __context,
                               IceInternal.Functional_VoidCallback __responseCb,
                               IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb);

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param __context The context map for the invocation.
     * @param __responseCb The asynchronous response callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @param __sentCb The asynchronous sent callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_ping(java.util.Map<String, String> __context,
                               IceInternal.Functional_VoidCallback __responseCb,
                               IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb,
                               IceInternal.Functional_BoolCallback __sentCb);

    /**
     * Completes the asynchronous ice_ping request.
     *
     * @param __result The asynchronous result.
     **/
    void end_ice_ping(AsyncResult __result);

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @return The Slice type IDs of the interfaces supported by the target object, in base-to-derived
     * order. The first element of the returned array is always <code>::Ice::Object</code>.
     **/
    String[] ice_ids();

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @param __context The context map for the invocation.
     * @return The Slice type IDs of the interfaces supported by the target object, in base-to-derived
     * order. The first element of the returned array is always <code>::Ice::Object</code>.
     **/
    String[] ice_ids(java.util.Map<String, String> __context);

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_ids();

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @param __context The context map for the invocation.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_ids(java.util.Map<String, String> __context);

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_ids(Callback __cb);

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @param __context The context map for the invocation.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_ids(java.util.Map<String, String> __context, Callback __cb);

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_ids(Callback_Object_ice_ids __cb);

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @param __context The context map for the invocation.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_ids(java.util.Map<String, String> __context, Callback_Object_ice_ids __cb);

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @param __responseCb The asynchronous response callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_ids(IceInternal.Functional_GenericCallback1<String[]> __responseCb,
                              IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb);

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @param __responseCb The asynchronous response callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @param __sentCb The asynchronous sent callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_ids(IceInternal.Functional_GenericCallback1<String[]> __responseCb,
                              IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb,
                              IceInternal.Functional_BoolCallback __sentCb);

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @param __context The context map for the invocation.
     * @param __responseCb The asynchronous response callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_ids(java.util.Map<String, String> __context,
                              IceInternal.Functional_GenericCallback1<String[]> __responseCb,
                              IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb);

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @param __context The context map for the invocation.
     * @param __responseCb The asynchronous response callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @param __sentCb The asynchronous sent callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_ids(java.util.Map<String, String> __context,
                              IceInternal.Functional_GenericCallback1<String[]> __responseCb,
                              IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb,
                              IceInternal.Functional_BoolCallback __sentCb);

    /**
     * Completes the asynchronous ice_ids request.
     *
     * @param __result The asynchronous result.
     * @return The Slice type IDs of the interfaces supported by the target object, in base-to-derived
     * order. The first element of the returned array is always <code>::Ice::Object</code>.
     **/
    String[] end_ice_ids(AsyncResult __result);

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @return The Slice type ID of the most-derived interface.
     **/
    String ice_id();

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @param __context The context map for the invocation.
     * @return The Slice type ID of the most-derived interface.
     **/
    String ice_id(java.util.Map<String, String> __context);

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_id();

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @param __context The context map for the invocation.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_id(java.util.Map<String, String> __context);

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_id(Callback __cb);

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @param __context The context map for the invocation.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_id(java.util.Map<String, String> __context, Callback __cb);

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_id(Callback_Object_ice_id __cb);

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @param __context The context map for the invocation.
     * @param __cb The asynchronous callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_id(java.util.Map<String, String> __context, Callback_Object_ice_id __cb);

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @param __responseCb The asynchronous response callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_id(IceInternal.Functional_GenericCallback1<String> __responseCb,
                             IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb);

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @param __responseCb The asynchronous response callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @param __sentCb The asynchronous sent callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_id(IceInternal.Functional_GenericCallback1<String> __responseCb,
                             IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb,
                             IceInternal.Functional_BoolCallback __sentCb);

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @param __context The context map for the invocation.
     * @param __responseCb The asynchronous response callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_id(java.util.Map<String, String> __context,
                             IceInternal.Functional_GenericCallback1<String> __responseCb,
                             IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb);

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @param __context The context map for the invocation.
     * @param __responseCb The asynchronous response callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @param __sentCb The asynchronous sent callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_id(java.util.Map<String, String> __context,
                             IceInternal.Functional_GenericCallback1<String> __responseCb,
                             IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb,
                             IceInternal.Functional_BoolCallback __sentCb);

    /**
     * Completes the asynchronous ice_id request.
     *
     * @param __result The asynchronous result.
     * @return The Slice type ID of the most-derived interface.
     **/
    String end_ice_id(AsyncResult __result);

    /**
     * Invokes an operation dynamically.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * @param outParams The encoded out-paramaters and return value
     * for the operation. The return value follows any out-parameters.
     * @return If the operation completed successfully, the return value
     * is <code>true</code>. If the operation raises a user exception,
     * the return value is <code>false</code>; in this case, <code>outParams</code>
     * contains the encoded user exception. If the operation raises a run-time exception,
     * it throws it directly.
     *
     * @see Blobject
     * @see OperationMode
     **/
    boolean ice_invoke(String operation, OperationMode mode, byte[] inParams, ByteSeqHolder outParams);

    /**
     * Invokes an operation dynamically.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * @param outParams The encoded out-paramaters and return value
     * for the operation. The return value follows any out-parameters.
     * @param __context The context map for the invocation.
     * @return If the operation completed successfully, the return value
     * is <code>true</code>. If the operation raises a user exception,
     * the return value is <code>false</code>; in this case, <code>outParams</code>
     * contains the encoded user exception. If the operation raises a run-time exception,
     * it throws it directly.
     *
     * @see Blobject
     * @see OperationMode
     **/
    boolean ice_invoke(String operation, OperationMode mode, byte[] inParams, ByteSeqHolder outParams,
                       java.util.Map<String, String> __context);

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
    AsyncResult begin_ice_invoke(String operation, OperationMode mode, byte[] inParams);

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
    AsyncResult begin_ice_invoke(String operation, OperationMode mode, byte[] inParams,
                                 java.util.Map<String, String> __context);

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
    AsyncResult begin_ice_invoke(String operation, OperationMode mode, byte[] inParams, Callback __cb);

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
    AsyncResult begin_ice_invoke(String operation, OperationMode mode, byte[] inParams,
                                 java.util.Map<String, String> __context, Callback __cb);

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
    AsyncResult begin_ice_invoke(String operation, OperationMode mode, byte[] inParams,
                                 Callback_Object_ice_invoke __cb);

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
    AsyncResult begin_ice_invoke(String operation, OperationMode mode, byte[] inParams,
                                 java.util.Map<String, String> __context, Callback_Object_ice_invoke __cb);

    public interface FunctionalCallback_Object_ice_invoke_Response
    {
        void apply(boolean result, byte[] outArgs);
    }

    /**
     * Invokes an operation dynamically and asynchronously.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * for the operation. The return value follows any out-parameters.
     * @param __responseCb The asynchronous response callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @param __sentCb The asynchronous sent callback object.
     * @return The asynchronous result object.
     *
     * @see Blobject
     * @see OperationMode
     **/
    AsyncResult begin_ice_invoke(String operation, OperationMode mode, byte[] inParams,
                                 FunctionalCallback_Object_ice_invoke_Response __responseCb,
                                 IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb,
                                 IceInternal.Functional_BoolCallback __sentCb);

    /**
     * Invokes an operation dynamically and asynchronously.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * for the operation. The return value follows any out-parameters.
     * @param __responseCb The asynchronous response callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @return The asynchronous result object.
     *
     * @see Blobject
     * @see OperationMode
     **/
    AsyncResult begin_ice_invoke(String operation, OperationMode mode, byte[] inParams,
                                 FunctionalCallback_Object_ice_invoke_Response __responseCb,
                                 IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb);

    /**
     * Invokes an operation dynamically and asynchronously.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * @param __context The context map for the invocation.
     * for the operation. The return value follows any out-parameters.
     * @param __responseCb The asynchronous response callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @param __sentCb The asynchronous sent callback object.
     * @return The asynchronous result object.
     *
     * @see Blobject
     * @see OperationMode
     **/
    AsyncResult begin_ice_invoke(String operation, OperationMode mode, byte[] inParams,
                                 java.util.Map<String, String> __context,
                                 FunctionalCallback_Object_ice_invoke_Response __responseCb,
                                 IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb,
                                 IceInternal.Functional_BoolCallback __sentCb);

    /**
     * Invokes an operation dynamically and asynchronously.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * @param __context The context map for the invocation.
     * for the operation. The return value follows any out-parameters.
     * @param __responseCb The asynchronous response callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @return The asynchronous result object.
     *
     * @see Blobject
     * @see OperationMode
     **/
    AsyncResult begin_ice_invoke(String operation, OperationMode mode, byte[] inParams,
                                 java.util.Map<String, String> __context,
                                 FunctionalCallback_Object_ice_invoke_Response __responseCb,
                                 IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb);

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
    boolean end_ice_invoke(ByteSeqHolder outParams, AsyncResult __result);

    /**
     * Returns the identity embedded in this proxy.
     *
     * @return The identity of the target object.
     **/
    Identity ice_getIdentity();

    /**
     * Creates a new proxy that is identical to this proxy, except for the identity.
     *
     * @param newIdentity The identity for the new proxy.
     * @return The proxy with the new identity.
     **/
    ObjectPrx ice_identity(Identity newIdentity);

    /**
     * Returns the per-proxy context for this proxy.
     *
     * @return The per-proxy context. If the proxy does not have a per-proxy (implicit) context, the return value
     * is <code>null</code>.
     **/
    java.util.Map<String, String> ice_getContext();

    /**
     * Creates a new proxy that is identical to this proxy, except for the per-proxy context.
     *
     * @param newContext The context for the new proxy.
     * @return The proxy with the new per-proxy context.
     **/
    ObjectPrx ice_context(java.util.Map<String, String> newContext);

    /**
     * Returns the facet for this proxy.
     *
     * @return The facet for this proxy. If the proxy uses the default facet, the return value is the empty string.
     **/
    String ice_getFacet();

    /**
     * Creates a new proxy that is identical to this proxy, except for the facet.
     *
     * @param newFacet The facet for the new proxy.
     * @return The proxy with the new facet.
     **/
    ObjectPrx ice_facet(String newFacet);

    /**
     * Returns the adapter ID for this proxy.
     *
     * @return The adapter ID. If the proxy does not have an adapter ID, the return value is the empty string.
     **/
    String ice_getAdapterId();

    /**
     * Creates a new proxy that is identical to this proxy, except for the adapter ID.
     *
     * @param newAdapterId The adapter ID for the new proxy.
     * @return The proxy with the new adapter ID.
     **/
    ObjectPrx ice_adapterId(String newAdapterId);

    /**
     * Returns the endpoints used by this proxy.
     *
     * @return The endpoints used by this proxy.
     *
     * @see Endpoint
     **/
    Endpoint[] ice_getEndpoints();

    /**
     * Creates a new proxy that is identical to this proxy, except for the endpoints.
     *
     * @param newEndpoints The endpoints for the new proxy.
     * @return The proxy with the new endpoints.
     **/
    ObjectPrx ice_endpoints(Endpoint[] newEndpoints);

    /**
     * Returns the locator cache timeout of this proxy.
     *
     * @return The locator cache timeout value (in seconds).
     *
     * @see Locator
     **/
    int ice_getLocatorCacheTimeout();

    /**
     * Returns the invocation timeout of this proxy.
     *
     * @return The invocation timeout value (in seconds).
     **/
    int ice_getInvocationTimeout();

    /**
     * Returns the connection id of this proxy.
     *
     * @return The connection id.
     *
     **/
    String ice_getConnectionId();

    /**
     * Creates a new proxy that is identical to this proxy, except for the locator cache timeout.
     *
     * @param newTimeout The new locator cache timeout (in seconds).
     *
     * @see Locator
     **/
    ObjectPrx ice_locatorCacheTimeout(int newTimeout);

    /**
     * Creates a new proxy that is identical to this proxy, except for the invocation timeout.
     *
     * @param newTimeout The new invocation timeout (in seconds).
     *
     **/
    ObjectPrx ice_invocationTimeout(int newTimeout);

    /**
     * Returns whether this proxy caches connections.
     *
     * @return <code>true</code> if this proxy caches connections; <code>false</code>, otherwise.
     **/
    boolean ice_isConnectionCached();

    /**
     * Creates a new proxy that is identical to this proxy, except for connection caching.
     *
     * @param newCache <code>true</code> if the new proxy should cache connections; <code>false</code>, otherwise.
     * @return The new proxy with the specified caching policy.
     **/
    ObjectPrx ice_connectionCached(boolean newCache);

    /**
     * Returns how this proxy selects endpoints (randomly or ordered).
     *
     * @return The endpoint selection policy.
     *
     * @see EndpointSelectionType
     **/
    EndpointSelectionType ice_getEndpointSelection();

    /**
     * Creates a new proxy that is identical to this proxy, except for the endpoint selection policy.
     *
     * @param newType The new endpoint selection policy.
     * @return The new proxy with the specified endpoint selection policy.
     *
     * @see EndpointSelectionType
     **/
    ObjectPrx ice_endpointSelection(EndpointSelectionType newType);

    /**
     * Returns whether this proxy uses only secure endpoints.
     *
     * @return <code>True</code> if this proxy communicates only via secure endpoints; <code>false</code>, otherwise.
     **/
    boolean ice_isSecure();

    /**
     * Creates a new proxy that is identical to this proxy, except for how it selects endpoints.
     *
     * @param b If <code>b</code> is <code>true</code>, only endpoints that use a secure transport are
     * used by the new proxy. If <code>b</code> is false, the returned proxy uses both secure and insecure
     * endpoints.
     * @return The new proxy with the specified selection policy.
     **/
    ObjectPrx ice_secure(boolean b);

    /**
     * Creates a new proxy that is identical to this proxy, except for the encoding used to marshal
     * parameters.
     *
     * @param e The encoding version to use to marshal requests parameters.
     * @return The new proxy with the specified encoding version.
     **/
    ObjectPrx ice_encodingVersion(Ice.EncodingVersion e);

    /**
     * Returns the encoding version used to marshal requests parameters.
     *
     * @return The encoding version.
     **/
    Ice.EncodingVersion ice_getEncodingVersion();

    /**
     * Returns whether this proxy prefers secure endpoints.
     *
     * @return <code>true</code> if the proxy always attempts to invoke via secure endpoints before it
     * attempts to use insecure endpoints; <code>false</code>, otherwise.
     **/
    boolean ice_isPreferSecure();

    /**
     * Creates a new proxy that is identical to this proxy, except for its endpoint selection policy.
     *
     * @param b If <code>b</code> is <code>true</code>, the new proxy will use secure endpoints for invocations
     * and only use insecure endpoints if an invocation cannot be made via secure endpoints. If <code>b</code> is
     * <code>false</code>, the proxy prefers insecure endpoints to secure ones.
     * @return The new proxy with the new endpoint selection policy.
     **/
    ObjectPrx ice_preferSecure(boolean b);

    /**
     * Returns the router for this proxy.
     *
     * @return The router for the proxy. If no router is configured for the proxy, the return value
     * is <code>null</code>.
     **/
    Ice.RouterPrx ice_getRouter();

    /**
     * Creates a new proxy that is identical to this proxy, except for the router.
     *
     * @param router The router for the new proxy.
     * @return The new proxy with the specified router.
     **/
    ObjectPrx ice_router(Ice.RouterPrx router);

    /**
     * Returns the locator for this proxy.
     *
     * @return The locator for this proxy. If no locator is configured, the return value is <code>null</code>.
     **/
    Ice.LocatorPrx ice_getLocator();

    /**
     * Creates a new proxy that is identical to this proxy, except for the locator.
     *
     * @param locator The locator for the new proxy.
     * @return The new proxy with the specified locator.
     **/
    ObjectPrx ice_locator(Ice.LocatorPrx locator);

    /**
     * Returns whether this proxy uses collocation optimization.
     *
     * @return <code>true</code> if the proxy uses collocation optimization; <code>false</code>, otherwise.
     **/
    boolean ice_isCollocationOptimized();

    /**
     * Creates a new proxy that is identical to this proxy, except for collocation optimization.
     *
     * @param b <code>true</code> if the new proxy enables collocation optimization; <code>false</code>, otherwise.
     * @return The new proxy the specified collocation optimization.
     **/
    ObjectPrx ice_collocationOptimized(boolean b);

    /**
     * Creates a new proxy that is identical to this proxy, but uses twoway invocations.
     *
     * @return A new proxy that uses twoway invocations.
     **/
    ObjectPrx ice_twoway();

    /**
     * Returns whether this proxy uses twoway invocations.
     * @return <code>true</code> if this proxy uses twoway invocations; <code>false</code>, otherwise.
     **/
    boolean ice_isTwoway();

    /**
     * Creates a new proxy that is identical to this proxy, but uses oneway invocations.
     *
     * @return A new proxy that uses oneway invocations.
     **/
    ObjectPrx ice_oneway();

    /**
     * Returns whether this proxy uses oneway invocations.
     * @return <code>true</code> if this proxy uses oneway invocations; <code>false</code>, otherwise.
     **/
    boolean ice_isOneway();

    /**
     * Creates a new proxy that is identical to this proxy, but uses batch oneway invocations.
     *
     * @return A new proxy that uses batch oneway invocations.
     **/
    ObjectPrx ice_batchOneway();

    /**
     * Returns whether this proxy uses batch oneway invocations.
     * @return <code>true</code> if this proxy uses batch oneway invocations; <code>false</code>, otherwise.
     **/
    boolean ice_isBatchOneway();

    /**
     * Creates a new proxy that is identical to this proxy, but uses datagram invocations.
     *
     * @return A new proxy that uses datagram invocations.
     **/
    ObjectPrx ice_datagram();

    /**
     * Returns whether this proxy uses datagram invocations.
     * @return <code>true</code> if this proxy uses datagram invocations; <code>false</code>, otherwise.
     **/
    boolean ice_isDatagram();

    /**
     * Creates a new proxy that is identical to this proxy, but uses batch datagram invocations.
     *
     * @return A new proxy that uses batch datagram invocations.
     **/
    ObjectPrx ice_batchDatagram();

    /**
     * Returns whether this proxy uses batch datagram invocations.
     * @return <code>true</code> if this proxy uses batch datagram invocations; <code>false</code>, otherwise.
     **/
    boolean ice_isBatchDatagram();

    /**
     * Creates a new proxy that is identical to this proxy, except for compression.
     *
     * @param co <code>true</code> enables compression for the new proxy; <code>false</code> disables compression.
     * @return A new proxy with the specified compression setting.
     **/
    ObjectPrx ice_compress(boolean co);

    /**
     * Creates a new proxy that is identical to this proxy, except for its timeout setting.
     *
     * @param t The timeout for the new proxy in milliseconds.
     * @return A new proxy with the specified timeout.
     **/
    ObjectPrx ice_timeout(int t);

    /**
     * Creates a new proxy that is identical to this proxy, except for its connection ID.
     *
     * @param connectionId The connection ID for the new proxy. An empty string removes the
     * connection ID.
     *
     * @return A new proxy with the specified connection ID.
     **/
    ObjectPrx ice_connectionId(String connectionId);

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
    Connection ice_getConnection();

    /**
     * Asynchronously gets the connection for this proxy. The call does not block.
     *
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_getConnection();

    /**
     * Asynchronously gets the connection for this proxy. The call does not block.
     *
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_getConnection(Callback __cb);

    /**
     * Asynchronously gets the connection for this proxy. The call does not block.
     *
     * @param __cb The callback object to notify the application when the operation is complete.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_getConnection(Callback_Object_ice_getConnection __cb);

    /**
     * Asynchronously gets the connection for this proxy. The call does not block.
     *
     * @param __responseCb The callback object to notify the application when there is a response available.
     * @param __exceptionCb The callback object to notify the application when an exception occurs while getting
     * the connection.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_getConnection(IceInternal.Functional_GenericCallback1<Ice.Connection> __responseCb,
                                        IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb);

    /**
     * Completes the asynchronous get connection.
     *
     * @param __result The asynchronous result.
     **/
    Ice.Connection end_ice_getConnection(AsyncResult __result);

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
    Connection ice_getCachedConnection();

    /**
     * Flushes any pending batched requests for this communicator. The call blocks until the flush is complete.
     **/
    void ice_flushBatchRequests();

    /**
     * Asynchronously flushes any pending batched requests for this communicator. The call does not block.
     *
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_flushBatchRequests();

    /**
     * Asynchronously flushes any pending batched requests for this communicator. The call does not block.
     *
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_flushBatchRequests(Callback __cb);

    /**
     * Asynchronously flushes any pending batched requests for this communicator. The call does not block.
     *
     * @param __cb The callback object to notify the application when the flush is complete.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_flushBatchRequests(Callback_Object_ice_flushBatchRequests __cb);

    /**
     * Asynchronously flushes any pending batched requests for this communicator. The call does not block.
     *
     * @param __responseCb The asynchronous completion callback object.
     * @param __exceptionCb The asynchronous exception callback object.
     * @param __sentCb The asynchronous sent callback object.
     * @return The asynchronous result object.
     **/
    AsyncResult begin_ice_flushBatchRequests(IceInternal.Functional_VoidCallback __responseCb,
                                             IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb,
                                             IceInternal.Functional_BoolCallback __sentCb);

    /**
     * Completes the asynchronous flush request.
     *
     * @param __result The asynchronous result.
     **/
    void end_ice_flushBatchRequests(AsyncResult __result);

    /**
     * Returns whether this proxy equals the passed object. Two proxies are equal if they are equal in all respects,
     * that is, if their object identity, endpoints timeout settings, and so on are all equal.
     *
     * @param r The object to compare this proxy with.
     * @return <code>true</code> if this proxy is equal to <code>r</code>; <code>false</code>, otherwise.
     **/
    @Override
    boolean equals(java.lang.Object r);

    void __write(OutputStream os);
}
