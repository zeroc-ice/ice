// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using IceUtilInternal;
using Ice.Instrumentation;

namespace Ice
{
    /// <summary>
    /// Delegate for a successful <code>ice_isA</code> invocation.
    /// <param name="ret__">True if the remote object supports the type, false otherwise.</param>
    /// </summary>
    public delegate void Callback_Object_ice_isA(bool ret__);

    /// <summary>
    /// Delegate for a successful <code>ice_ids</code> invocation.
    /// <param name="ret__">The array of Slice type ids supported by the remote object.</param>
    /// </summary>
    public delegate void Callback_Object_ice_ids(string[] ret__);

    /// <summary>
    /// Delegate for a successful <code>ice_id</code> invocation.
    /// <param name="ret__">The Slice type id of the most-derived interface supported by the remote object.</param>
    /// </summary>
    public delegate void Callback_Object_ice_id(string ret__);

    /// <summary>
    /// Delegate for a successful <code>ice_ping</code> invocation.
    /// </summary>
    public delegate void Callback_Object_ice_ping();

    /// <summary>
    /// Delegate for a successful <code>ice_invoke</code> invocation.
    /// <param name="ret__">True if the invocation succeeded, or false if the invocation
    /// raised a user exception.</param>
    /// <param name="outEncaps">The encoded out-parameters or user exception.</param>
    /// </summary>
    public delegate void Callback_Object_ice_invoke(bool ret__, byte[] outEncaps);

    /// <summary>
    /// Callback object for Blobject AMI invocations.
    /// </summary>
    public abstract class AMI_Object_ice_invoke : AMICallbackBase
    {
         /// <summary>
         /// The Ice run time calls <code>ice_response</code> when an asynchronous operation invocation
         /// completes successfully or raises a user exception.
         /// </summary>
         /// <param name="ok">Indicates the result of the invocation. If true, the operation
         /// completed succesfully; if false, the operation raised a user exception.</param>
         /// <param name="outEncaps">Contains the encoded out-parameters of the operation (if any) if ok
         /// is true; otherwise, if ok is false, contains the
         /// encoded user exception raised by the operation.</param>
        public abstract void ice_response(bool ok, byte[] outEncaps);

        public void response__(bool ok, byte[] outEncaps)
        {
            ice_response(ok, outEncaps);
        }
    }

    /// <summary>
    /// Callback object for ObjectPrx.ice_flushBatchRequests_async.
    /// </summary>
    public abstract class AMI_Object_ice_flushBatchRequests : AMICallbackBase
    {
        //
        // Subclass must override ice_exception, which is inherited from AMICallbackBase.
        //
    }

    /// <summary>
    /// Base interface of all object proxies.
    /// </summary>
    public interface ObjectPrx
    {
        /// <summary>
        /// Returns the communicator that created this proxy.
        /// </summary>
        /// <returns>The communicator that created this proxy.</returns>
        Communicator ice_getCommunicator();

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="id__">The type ID of the Slice interface to test against.</param>
        /// <returns>True if the target object has the interface specified by id__ or derives
        /// from the interface specified by id__.</returns>
        bool ice_isA(string id__);

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="id__">The type ID of the Slice interface to test against.</param>
        /// <param name="context__">The context dictionary for the invocation.</param>
        /// <returns>True if the target object has the interface specified by id__ or derives
        /// from the interface specified by id__.</returns>
        bool ice_isA(string id__, Dictionary<string, string> context__);

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult<Callback_Object_ice_isA> begin_ice_isA(string id);

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="context__">The context dictionary for the invocation.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult<Callback_Object_ice_isA> begin_ice_isA(string id, Dictionary<string, string> context__);

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="cb__">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie__">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_isA(string id, AsyncCallback cb__, object cookie__);

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="context__">The context dictionary for the invocation.</param>
        /// <param name="cb__">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie__">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_isA(string id, Dictionary<string, string> context__, AsyncCallback cb__, object cookie__);

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="r__">The asynchronous result object returned by <code>begin_ice_isA</code>.</param>
        /// <returns>True if the object supports the Slice interface, false otherwise.</returns>
        bool end_ice_isA(AsyncResult r__);

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        void ice_ping();

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context__">The context dictionary for the invocation.</param>
        void ice_ping(Dictionary<string, string> context__);

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult<Callback_Object_ice_ping> begin_ice_ping();

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context__">The context dictionary for the invocation.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult<Callback_Object_ice_ping> begin_ice_ping(Dictionary<string, string> context__);

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="cb__">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie__">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_ping(AsyncCallback cb__, object cookie__);

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context__">The context dictionary for the invocation.</param>
        /// <param name="cb__">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie__">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_ping(Dictionary<string, string> context__, AsyncCallback cb__, object cookie__);

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="r__">The asynchronous result object returned by <code>begin_ice_ping</code>.</param>
        void end_ice_ping(AsyncResult r__);

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <returns>The Slice type IDs of the interfaces supported by the target object, in base-to-derived
        /// order. The first element of the returned array is always ::Ice::Object.</returns>
        string[] ice_ids();

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="context__">The context dictionary for the invocation.</param>
        /// <returns>The Slice type IDs of the interfaces supported by the target object, in base-to-derived
        /// order. The first element of the returned array is always ::Ice::Object.</returns>
        string[] ice_ids(Dictionary<string, string> context__);

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult<Callback_Object_ice_ids> begin_ice_ids();

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="context__">The context dictionary for the invocation.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult<Callback_Object_ice_ids> begin_ice_ids(Dictionary<string, string> context__);

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="cb__">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie__">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_ids(AsyncCallback cb__, object cookie__);

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="context__">The context dictionary for the invocation.</param>
        /// <param name="cb__">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie__">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_ids(Dictionary<string, string> context__, AsyncCallback cb__, object cookie__);

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="r__">The asynchronous result object returned by <code>begin_ice_ids</code>.</param>
        /// <returns>The Slice type IDs of the interfaces supported by the target object, in base-to-derived
        /// order. The first element of the returned array is always ::Ice::Object.</returns>
        string[] end_ice_ids(AsyncResult r__);

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        string ice_id();

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="context__">The context dictionary for the invocation.</param>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        string ice_id(Dictionary<string, string> context__);

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult<Callback_Object_ice_id> begin_ice_id();

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="context__">The context dictionary for the invocation.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult<Callback_Object_ice_id> begin_ice_id(Dictionary<string, string> context__);

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="cb__">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie__">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_id(AsyncCallback cb__, object cookie__);

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="context__">The context dictionary for the invocation.</param>
        /// <param name="cb__">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie__">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_id(Dictionary<string, string> context__, AsyncCallback cb__, object cookie__);

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="r__">The asynchronous result object returned by <code>begin_ice_id</code>.</param>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        string end_ice_id(AsyncResult r__);

        /// <summary>
        /// Invokes an operation dynamically.
        /// </summary>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <param name="outEncaps">The encoded out-paramaters and return value
        /// for the operation. The return value follows any out-parameters.</param>
        /// <returns>If the operation completed successfully, the return value
        /// is true. If the operation raises a user exception,
        /// the return value is false; in this case, outEncaps
        /// contains the encoded user exception. If the operation raises a run-time exception,
        /// it throws it directly.</returns>
        bool ice_invoke(string operation, OperationMode mode, byte[] inEncaps, out byte[] outEncaps);

        /// <summary>
        /// Invokes an operation dynamically.
        /// </summary>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <param name="outEncaps">The encoded out-paramaters and return value
        /// for the operation. The return value follows any out-parameters.</param>
        /// <param name="context__">The context dictionary for the invocation.</param>
        /// <returns>If the operation completed successfully, the return value
        /// is true. If the operation raises a user exception,
        /// the return value is false; in this case, outEncaps
        /// contains the encoded user exception. If the operation raises a run-time exception,
        /// it throws it directly.</returns>
        bool ice_invoke(string operation, OperationMode mode, byte[] inEncaps, out byte[] outEncaps,
                        Dictionary<string, string> context__);

        /// <summary>
        /// Invokes an operation dynamically and asynchronously.
        /// </summary>
        /// <param name="cb">The callback object to notify when the operation completes.</param>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <returns> If the operation was invoked synchronously (because there
        /// was no need to queue the request), the return value is true;
        /// otherwise, if the invocation was queued, the return value is false.</returns>
        bool ice_invoke_async(AMI_Object_ice_invoke cb, string operation, OperationMode mode, byte[] inEncaps);

        /// <summary>
        /// Invokes an operation dynamically and asynchronously.
        /// </summary>
        /// <param name="cb">The callback object to notify when the operation completes.</param>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns> If the operation was invoked synchronously (because there
        /// was no need to queue the request), the return value is true;
        /// otherwise, if the invocation was queued, the return value is false.</returns>
        bool ice_invoke_async(AMI_Object_ice_invoke cb, string operation, OperationMode mode, byte[] inEncaps,
                              Dictionary<string, string> context);

        /// <summary>
        /// Invokes an operation dynamically.
        /// </summary>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult<Callback_Object_ice_invoke> begin_ice_invoke(string operation, OperationMode mode,
                                                                    byte[] inEncaps);

        /// <summary>
        /// Invokes an operation dynamically.
        /// </summary>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <param name="context__">The context dictionary for the invocation.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult<Callback_Object_ice_invoke> begin_ice_invoke(string operation, OperationMode mode,
                                                                    byte[] inEncaps,
                                                                    Dictionary<string, string> context__);

        /// <summary>
        /// Invokes an operation dynamically.
        /// </summary>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <param name="cb__">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie__">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_invoke(string operation, OperationMode mode, byte[] inEncaps, AsyncCallback cb__,
                                     object cookie__);

        /// <summary>
        /// Invokes an operation dynamically.
        /// </summary>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <param name="context__">The context dictionary for the invocation.</param>
        /// <param name="cb__">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie__">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_invoke(string operation, OperationMode mode, byte[] inEncaps,
                                     Dictionary<string, string> context__, AsyncCallback cb__, object cookie__);

        /// <summary>
        /// Completes a dynamic invocation.
        /// </summary>
        /// <param name="outEncaps">The encoded out parameters or user exception.</param>
        /// <param name="r__">The asynchronous result object returned by <code>begin_ice_invoke</code>.</param>
        /// <returns>If the operation completed successfully, the return value
        /// is true. If the operation raises a user exception,
        /// the return value is false; in this case, outEncaps
        /// contains the encoded user exception. If the operation raises a run-time exception,
        /// it throws it directly.</returns>
        bool end_ice_invoke(out byte[] outEncaps, AsyncResult r__);

        /// <summary>
        /// Returns the identity embedded in this proxy.
        /// <returns>The identity of the target object.</returns>
        /// </summary>
        Identity ice_getIdentity();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the per-proxy context.
        /// <param name="newIdentity">The identity for the new proxy.</param>
        /// <returns>The proxy with the new identity.</returns>
        /// </summary>
        ObjectPrx ice_identity(Identity newIdentity);

        /// <summary>
        /// Returns the per-proxy context for this proxy.
        /// </summary>
        /// <returns>The per-proxy context. If the proxy does not have a per-proxy (implicit) context, the return value
        /// is null.</returns>
        Dictionary<string, string> ice_getContext();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the per-proxy context.
        /// </summary>
        /// <param name="newContext">The context for the new proxy.</param>
        /// <returns>The proxy with the new per-proxy context.</returns>
        ObjectPrx ice_context(Dictionary<string, string> newContext);

        /// <summary>
        /// Returns the facet for this proxy.
        /// </summary>
        /// <returns>The facet for this proxy. If the proxy uses the default facet, the return value is the
        /// empty string.</returns>
        string ice_getFacet();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the facet.
        /// </summary>
        /// <param name="newFacet">The facet for the new proxy.</param>
        /// <returns>The proxy with the new facet.</returns>
        ObjectPrx ice_facet(string newFacet);

        /// <summary>
        /// Returns the adapter ID for this proxy.
        /// </summary>
        /// <returns>The adapter ID. If the proxy does not have an adapter ID, the return value is the
        /// empty string.</returns>
        string ice_getAdapterId();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the adapter ID.
        /// </summary>
        /// <param name="newAdapterId">The adapter ID for the new proxy.</param>
        /// <returns>The proxy with the new adapter ID.</returns>
        ObjectPrx ice_adapterId(string newAdapterId);

        /// <summary>
        /// Returns the endpoints used by this proxy.
        /// </summary>
        /// <returns>The endpoints used by this proxy.</returns>
        Endpoint[] ice_getEndpoints();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the endpoints.
        /// </summary>
        /// <param name="newEndpoints">The endpoints for the new proxy.</param>
        /// <returns>The proxy with the new endpoints.</returns>
        ObjectPrx ice_endpoints(Endpoint[] newEndpoints);

        /// <summary>
        /// Returns the locator cache timeout of this proxy.
        /// </summary>
        /// <returns>The locator cache timeout value (in seconds).</returns>
        int ice_getLocatorCacheTimeout();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the locator cache timeout.
        /// </summary>
        /// <param name="timeout">The new locator cache timeout (in seconds).</param>
        ObjectPrx ice_locatorCacheTimeout(int timeout);

        /// <summary>
        /// Returns whether this proxy caches connections.
        /// </summary>
        /// <returns>True if this proxy caches connections; false, otherwise.</returns>
        bool ice_isConnectionCached();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for connection caching.
        /// </summary>
        /// <param name="newCache">True if the new proxy should cache connections; false, otherwise.</param>
        /// <returns>The new proxy with the specified caching policy.</returns>
        ObjectPrx ice_connectionCached(bool newCache);

        /// <summary>
        /// Returns how this proxy selects endpoints (randomly or ordered).
        /// </summary>
        /// <returns>The endpoint selection policy.</returns>
        EndpointSelectionType ice_getEndpointSelection();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the endpoint selection policy.
        /// </summary>
        /// <param name="newType">The new endpoint selection policy.</param>
        /// <returns>The new proxy with the specified endpoint selection policy.</returns>
        ObjectPrx ice_endpointSelection(EndpointSelectionType newType);

        /// <summary>
        /// Returns whether this proxy communicates only via secure endpoints.
        /// </summary>
        /// <returns>True if this proxy communicates only vi secure endpoints; false, otherwise.</returns>
        bool ice_isSecure();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for how it selects endpoints.
        /// </summary>
        /// <param name="b"> If b is true, only endpoints that use a secure transport are
        /// used by the new proxy. If b is false, the returned proxy uses both secure and insecure
        /// endpoints.</param>
        /// <returns>The new proxy with the specified selection policy.</returns>
        ObjectPrx ice_secure(bool b);

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the encoding used to marshal
        /// parameters.
        /// </summary>
        /// <param name="e">The encoding version to use to marshal requests parameters.</param>
        /// <returns>The new proxy with the specified encoding version.</returns>
        ObjectPrx ice_encodingVersion(Ice.EncodingVersion e);

        /// <summary>Returns the encoding version used to marshal requests parameters.</summary>
        /// <returns>The encoding version.</returns>
        Ice.EncodingVersion ice_getEncodingVersion();

        /// <summary>
        /// Returns whether this proxy prefers secure endpoints.
        /// </summary>
        /// <returns>True if the proxy always attempts to invoke via secure endpoints before it
        /// attempts to use insecure endpoints; false, otherwise.</returns>
        bool ice_isPreferSecure();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for its endpoint selection policy.
        /// </summary>
        /// <param name="b">If b is true, the new proxy will use secure endpoints for invocations
        /// and only use insecure endpoints if an invocation cannot be made via secure endpoints. If b is
        /// false, the proxy prefers insecure endpoints to secure ones.</param>
        /// <returns>The new proxy with the new endpoint selection policy.</returns>
        ObjectPrx ice_preferSecure(bool b);

        /// <summary>
        /// Returns the router for this proxy.
        /// </summary>
        /// <returns>The router for the proxy. If no router is configured for the proxy, the return value
        /// is null.</returns>
        Ice.RouterPrx ice_getRouter();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the router.
        /// </summary>
        /// <param name="router">The router for the new proxy.</param>
        /// <returns>The new proxy with the specified router.</returns>
        ObjectPrx ice_router(Ice.RouterPrx router);

        /// <summary>
        /// Returns the locator for this proxy.
        /// </summary>
        /// <returns>The locator for this proxy. If no locator is configured, the return value is null.</returns>
        Ice.LocatorPrx ice_getLocator();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the locator.
        /// </summary>
        /// <param name="locator">The locator for the new proxy.</param>
        /// <returns>The new proxy with the specified locator.</returns>
        ObjectPrx ice_locator(Ice.LocatorPrx locator);

        /// <summary>
        /// Returns whether this proxy uses collocation optimization.
        /// </summary>
        /// <returns>True if the proxy uses collocation optimization; false, otherwise.</returns>
        bool ice_isCollocationOptimized();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for collocation optimization.
        /// </summary>
        /// <param name="b">True if the new proxy enables collocation optimization; false, otherwise.</param>
        /// <returns>The new proxy the specified collocation optimization.</returns>
        ObjectPrx ice_collocationOptimized(bool b);

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, but uses twoway invocations.
        /// </summary>
        /// <returns>A new proxy that uses twoway invocations.</returns>
        ObjectPrx ice_twoway();

        /// <summary>
        /// Returns whether this proxy uses twoway invocations.
        /// </summary>
        /// <returns>True if this proxy uses twoway invocations; false, otherwise.</returns>
        bool ice_isTwoway();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, but uses oneway invocations.
        /// </summary>
        /// <returns>A new proxy that uses oneway invocations.</returns>
        ObjectPrx ice_oneway();

        /// <summary>
        /// Returns whether this proxy uses oneway invocations.
        /// </summary>
        /// <returns>True if this proxy uses oneway invocations; false, otherwise.</returns>
        bool ice_isOneway();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, but uses batch oneway invocations.
        /// </summary>
        /// <returns>A new proxy that uses batch oneway invocations.</returns>
        ObjectPrx ice_batchOneway();

        /// <summary>
        /// Returns whether this proxy uses batch oneway invocations.
        /// </summary>
        /// <returns>True if this proxy uses batch oneway invocations; false, otherwise.</returns>
        bool ice_isBatchOneway();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, but uses datagram invocations.
        /// </summary>
        /// <returns>A new proxy that uses datagram invocations.</returns>
        ObjectPrx ice_datagram();

        /// <summary>
        /// Returns whether this proxy uses datagram invocations.
        /// </summary>
        /// <returns>True if this proxy uses datagram invocations; false, otherwise.</returns>
        bool ice_isDatagram();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, but uses batch datagram invocations.
        /// </summary>
        /// <returns>A new proxy that uses batch datagram invocations.</returns>
        ObjectPrx ice_batchDatagram();

        /// <summary>
        /// Returns whether this proxy uses batch datagram invocations.
        /// </summary>
        /// <returns>True if this proxy uses batch datagram invocations; false, otherwise.</returns>
        bool ice_isBatchDatagram();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for compression.
        /// </summary>
        /// <param name="co">True enables compression for the new proxy; false disables compression.</param>
        /// <returns>A new proxy with the specified compression setting.</returns>
        ObjectPrx ice_compress(bool co);

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for its timeout setting.
        /// </summary>
        /// <param name="t">The timeout for the new proxy in milliseconds.</param>
        /// <returns>A new proxy with the specified timeout.</returns>
        ObjectPrx ice_timeout(int t);

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for its connection ID.
        /// </summary>
        /// <param name="connectionId">The connection ID for the new proxy. An empty string removes the
        /// connection ID.</param>
        /// <returns>A new proxy with the specified connection ID.</returns>
        ObjectPrx ice_connectionId(string connectionId);

        /// <summary>
        /// Returns the connection id of this proxy.
        /// </summary>
        /// <returns>The connection id.</returns>
        string ice_getConnectionId();

        /// <summary>
        /// Returns the Connection for this proxy. If the proxy does not yet have an established connection,
        /// it first attempts to create a connection.
        /// </summary>
        /// <returns>The Connection for this proxy.</returns>
        /// <exception name="CollocationOptimizationException">If the proxy uses collocation optimization and denotes a
        /// collocated object.</exception>
        Connection ice_getConnection();

        /// <summary>
        /// Returns the cached Connection for this proxy. If the proxy does not yet have an established
        /// connection, it does not attempt to create a connection.
        /// </summary>
        /// <returns>The cached Connection for this proxy (null if the proxy does not have
        /// an established connection).</returns>
        /// <exception name="CollocationOptimizationException">If the proxy uses collocation optimization and denotes a
        /// collocated object.</exception>
        Connection ice_getCachedConnection();

        /// <summary>
        /// Flushes any pending batched requests for this communicator. The call blocks until the flush is complete.
        /// </summary>
        void ice_flushBatchRequests();

        /// <summary>
        /// Asynchronously flushes any pending batched requests for this communicator. The call does not block.
        /// </summary>
        /// <param name="cb">The callback object to notify the application when the flush is complete.</param>
        /// <returns>True if the requests were flushed immediately without blocking; false
        /// if the requests could not be flushed immediately.</returns>
        bool ice_flushBatchRequests_async(AMI_Object_ice_flushBatchRequests cb);

        AsyncResult begin_ice_flushBatchRequests();
        AsyncResult begin_ice_flushBatchRequests(AsyncCallback cb__, object cookie__);

        void end_ice_flushBatchRequests(AsyncResult r__);
    }

    /// <summary>
    /// Base class of all object proxies.
    /// </summary>
    public class ObjectPrxHelperBase : ObjectPrx
    {
        /// <summary>
        /// Returns a hash code for this proxy.
        /// </summary>
        /// <returns>The hash code.</returns>
        public override int GetHashCode()
        {
            return _reference.GetHashCode();
        }

        /// <summary>
        /// Returns the communicator that created this proxy.
        /// </summary>
        /// <returns>The communicator that created this proxy.</returns>
        public Communicator ice_getCommunicator()
        {
            return _reference.getCommunicator();
        }

        /// <summary>
        /// Returns the stringified form of this proxy.
        /// </summary>
        /// <returns>The stringified proxy.</returns>
        public override string ToString()
        {
            return _reference.ToString();
        }

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="id__">The type ID of the Slice interface to test against.</param>
        /// <returns>True if the target object has the interface specified by id__ or derives
        /// from the interface specified by id__.</returns>
        public bool ice_isA(string id__)
        {
            return ice_isA(id__, null, false);
        }

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="id__">The type ID of the Slice interface to test against.</param>
        /// <param name="context__">The context dictionary for the invocation.</param>
        /// <returns>True if the target object has the interface specified by id__ or derives
        /// from the interface specified by id__.</returns>
        public bool ice_isA(string id__, Dictionary<string, string> context__)
        {
            return ice_isA(id__, context__, true);
        }

        private bool ice_isA(string id__, Dictionary<string, string> context__, bool explicitContext__)
        {
            if(explicitContext__ && context__ == null)
            {
                context__ = emptyContext_;
            }

            InvocationObserver observer__ = IceInternal.ObserverHelper.get(this, __ice_isA_name, context__);
            int cnt__ = 0;
            try
            {
                while(true)
                {
                    ObjectDel_ del__ = null;
                    try
                    {
                        checkTwowayOnly__(__ice_isA_name);
                        del__ = getDelegate__(false);
                        return del__.ice_isA(id__, context__, observer__);
                    }
                    catch(IceInternal.LocalExceptionWrapper ex__)
                    {
                        handleExceptionWrapperRelaxed__(del__, ex__, true, ref cnt__, observer__);
                    }
                    catch(LocalException ex__)
                    {
                        handleException__(del__, ex__, true, ref cnt__, observer__);
                    }
                }
            }
            finally
            {
                if(observer__ != null)
                {
                    observer__.detach();
                }
            }
        }

        public AsyncResult<Callback_Object_ice_isA> begin_ice_isA(string id)
        {
            return begin_ice_isA(id, null, false, null, null);
        }

        public AsyncResult<Callback_Object_ice_isA> begin_ice_isA(string id, Dictionary<string, string> context__)
        {
            return begin_ice_isA(id, context__, true, null, null);
        }

        public AsyncResult begin_ice_isA(string id, AsyncCallback cb__, object cookie__)
        {
            return begin_ice_isA(id, null, false, cb__, cookie__);
        }

        public AsyncResult begin_ice_isA(string id, Dictionary<string, string> context__, AsyncCallback cb__,
                                         object cookie__)
        {
            return begin_ice_isA(id, null, false, cb__, cookie__);
        }

        internal const string __ice_isA_name = "ice_isA";

        public bool end_ice_isA(AsyncResult r__)
        {
            IceInternal.OutgoingAsync outAsync__ = (IceInternal.OutgoingAsync)r__;
            IceInternal.OutgoingAsync.check__(outAsync__, this, __ice_isA_name);
            bool ok = outAsync__.wait__();
            try
            {
                if(!ok)
                {
                    try
                    {
                        outAsync__.throwUserException__();
                    }
                    catch(Ice.UserException ex__)
                    {
                        throw new Ice.UnknownUserException(ex__.ice_name(), ex__);
                    }
                }
                bool ret__;
                IceInternal.BasicStream is__ = outAsync__.startReadParams__();
                ret__ = is__.readBool();
                outAsync__.endReadParams__();
                return ret__;
            }
            catch(Ice.LocalException ex)
            {
                InvocationObserver obsv__ = outAsync__.getObserver__();
                if(obsv__ != null)
                {
                    obsv__.failed(ex.ice_name());
                }
                throw ex;
            }
        }

        private AsyncResult<Callback_Object_ice_isA> begin_ice_isA(string id, Dictionary<string, string> context__,
                                                                   bool explicitContext__, Ice.AsyncCallback cb__,
                                                                   object cookie__)
        {
            IceInternal.TwowayOutgoingAsync<Callback_Object_ice_isA> result__ =
                new IceInternal.TwowayOutgoingAsync<Callback_Object_ice_isA>(this, __ice_isA_name, ice_isA_completed__,
                                                                             cookie__);
            if(cb__ != null)
            {
                result__.whenCompletedWithAsyncCallback(cb__);
            }
            checkAsyncTwowayOnly__(__ice_isA_name);

            try
            {
                result__.prepare__(__ice_isA_name, OperationMode.Nonmutating, context__, explicitContext__);
                IceInternal.BasicStream os__ = result__.startWriteParams__(FormatType.DefaultFormat);
                os__.writeString(id);
                result__.endWriteParams__();
                result__.send__(true);
            }
            catch(Ice.LocalException ex__)
            {
                result__.exceptionAsync__(ex__);
            }
            return result__;
        }

        private void ice_isA_completed__(AsyncResult r__, Callback_Object_ice_isA cb__, Ice.ExceptionCallback excb__)
        {
            bool ret__;
            try
            {
                ret__ = end_ice_isA(r__);
            }
            catch(Ice.Exception ex__)
            {
                if(excb__ != null)
                {
                    excb__(ex__);
                }
                return;
            }
            if(cb__ != null)
            {
                cb__(ret__);
            }
        }

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        public void ice_ping()
        {
            ice_ping(null, false);
        }

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context__">The context dictionary for the invocation.</param>
        public void ice_ping(Dictionary<string, string> context__)
        {
            ice_ping(context__, true);
        }

        private void ice_ping(Dictionary<string, string> context__, bool explicitContext__)
        {
            if(explicitContext__ && context__ == null)
            {
                context__ = emptyContext_;
            }

            InvocationObserver observer__ = IceInternal.ObserverHelper.get(this, __ice_ping_name, context__);
            int cnt__ = 0;
            try
            {
                while(true)
                {
                    ObjectDel_ del__ = null;
                    try
                    {
                        del__ = getDelegate__(false);
                        del__.ice_ping(context__, observer__);
                        return;
                    }
                    catch(IceInternal.LocalExceptionWrapper ex__)
                    {
                        handleExceptionWrapperRelaxed__(del__, ex__, true, ref cnt__, observer__);
                    }
                    catch(LocalException ex__)
                    {
                        handleException__(del__, ex__, true, ref cnt__, observer__);
                    }
                }
            }
            finally
            {
                if(observer__ != null)
                {
                    observer__.detach();
                }
            }
        }

        public AsyncResult<Callback_Object_ice_ping> begin_ice_ping()
        {
            return begin_ice_ping(null, false, null, null);
        }

        public AsyncResult<Callback_Object_ice_ping> begin_ice_ping(Dictionary<string, string> context__)
        {
            return begin_ice_ping(context__, true, null, null);
        }

        public AsyncResult begin_ice_ping(AsyncCallback cb__, object cookie__)
        {
            return begin_ice_ping(null, false, cb__, cookie__);
        }

        public AsyncResult begin_ice_ping(Dictionary<string, string> context__, AsyncCallback cb__, object cookie__)
        {
            return begin_ice_ping(null, false, cb__, cookie__);
        }

        internal const string __ice_ping_name = "ice_ping";

        public void end_ice_ping(AsyncResult r__)
        {
            end__(r__, __ice_ping_name);
        }

        private AsyncResult<Callback_Object_ice_ping> begin_ice_ping(Dictionary<string, string> context__,
                                                                 bool explicitContext__,
                                                                 Ice.AsyncCallback cb__,
                                                                 object cookie__)
        {
            IceInternal.OnewayOutgoingAsync<Callback_Object_ice_ping> result__ =
                new IceInternal.OnewayOutgoingAsync<Callback_Object_ice_ping>(this, __ice_ping_name,
                                                                              ice_ping_completed__, cookie__);
            if(cb__ != null)
            {
                result__.whenCompletedWithAsyncCallback(cb__);
            }

            try
            {
                result__.prepare__(__ice_ping_name, OperationMode.Nonmutating, context__, explicitContext__);
                result__.writeEmptyParams__();
                result__.send__(true);
            }
            catch(Ice.LocalException ex__)
            {
                result__.exceptionAsync__(ex__);
            }
            return result__;
        }

        private void ice_ping_completed__(Callback_Object_ice_ping cb)
        {
            if(cb != null)
            {
                cb();
            }
        }

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <returns>The Slice type IDs of the interfaces supported by the target object, in base-to-derived
        /// order. The first element of the returned array is always ::Ice::Object.</returns>
        public string[] ice_ids()
        {
            return ice_ids(null, false);
        }

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="context__">The context dictionary for the invocation.</param>
        /// <returns>The Slice type IDs of the interfaces supported by the target object, in base-to-derived
        /// order. The first element of the returned array is always ::Ice::Object.</returns>
        public string[] ice_ids(Dictionary<string, string> context__)
        {
            return ice_ids(context__, true);
        }

        private string[] ice_ids(Dictionary<string, string> context__, bool explicitContext__)
        {
            if(explicitContext__ && context__ == null)
            {
                context__ = emptyContext_;
            }

            InvocationObserver observer__ = IceInternal.ObserverHelper.get(this, __ice_ids_name, context__);
            int cnt__ = 0;
            try
            {
                while(true)
                {
                    ObjectDel_ del__ = null;
                    try
                    {
                        checkTwowayOnly__(__ice_ids_name);
                        del__ = getDelegate__(false);
                        return del__.ice_ids(context__, observer__);
                    }
                    catch(IceInternal.LocalExceptionWrapper ex__)
                    {
                        handleExceptionWrapperRelaxed__(del__, ex__, true, ref cnt__, observer__);
                    }
                    catch(LocalException ex__)
                    {
                        handleException__(del__, ex__, true, ref cnt__, observer__);
                    }
                }
            }
            finally
            {
                if(observer__ != null)
                {
                    observer__.detach();
                }
            }
        }

        public AsyncResult<Callback_Object_ice_ids> begin_ice_ids()
        {
            return begin_ice_ids(null, false, null, null);
        }

        public AsyncResult<Callback_Object_ice_ids> begin_ice_ids(Dictionary<string, string> context__)
        {
            return begin_ice_ids(context__, true, null, null);
        }

        public AsyncResult begin_ice_ids(AsyncCallback cb__, object cookie__)
        {
            return begin_ice_ids(null, false, cb__, cookie__);
        }

        public AsyncResult begin_ice_ids(Dictionary<string, string> context__, AsyncCallback cb__, object cookie__)
        {
            return begin_ice_ids(null, false, cb__, cookie__);
        }

        internal const string __ice_ids_name = "ice_ids";

        public string[] end_ice_ids(AsyncResult r__)
        {
            IceInternal.OutgoingAsync outAsync__ = (IceInternal.OutgoingAsync)r__;
            IceInternal.OutgoingAsync.check__(outAsync__, this, __ice_ids_name);
            bool ok = outAsync__.wait__();
            try
            {
                if(!ok)
                {
                    try
                    {
                        outAsync__.throwUserException__();
                    }
                    catch(Ice.UserException ex__)
                    {
                        throw new Ice.UnknownUserException(ex__.ice_name(), ex__);
                    }
                }
                string[] ret__;
                IceInternal.BasicStream is__ = outAsync__.startReadParams__();
                ret__ = is__.readStringSeq();
                outAsync__.endReadParams__();
                return ret__;
            }
            catch(Ice.LocalException ex)
            {
                InvocationObserver obsv__ = outAsync__.getObserver__();
                if(obsv__ != null)
                {
                    obsv__.failed(ex.ice_name());
                }
                throw ex;
            }
        }

        private AsyncResult<Callback_Object_ice_ids> begin_ice_ids(Dictionary<string, string> context__,
                                                                      bool explicitContext__,
                                                                      Ice.AsyncCallback cb__,
                                                                      object cookie__)
        {
            IceInternal.TwowayOutgoingAsync<Callback_Object_ice_ids> result__ =
                new IceInternal.TwowayOutgoingAsync<Callback_Object_ice_ids>(this, __ice_ids_name, ice_ids_completed__,
                                                                             cookie__);
            if(cb__ != null)
            {
                result__.whenCompletedWithAsyncCallback(cb__);
            }
            checkAsyncTwowayOnly__(__ice_ids_name);

            try
            {
                result__.prepare__(__ice_ids_name, OperationMode.Nonmutating, context__, explicitContext__);
                result__.writeEmptyParams__();
                result__.send__(true);
            }
            catch(Ice.LocalException ex__)
            {
                result__.exceptionAsync__(ex__);
            }
            return result__;
        }

        private void ice_ids_completed__(AsyncResult r__, Callback_Object_ice_ids cb__, Ice.ExceptionCallback excb__)
        {
            string[] ret__;
            try
            {
                ret__ = end_ice_ids(r__);
            }
            catch(Ice.Exception ex__)
            {
                if(excb__ != null)
                {
                    excb__(ex__);
                }
                return;
            }
            if(cb__ != null)
            {
                cb__(ret__);
            }
        }

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        public string ice_id()
        {
            return ice_id(null, false);
        }

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="context__">The context dictionary for the invocation.</param>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        public string ice_id(Dictionary<string, string> context__)
        {
             return ice_id(context__, true);
        }

        private string ice_id(Dictionary<string, string> context__, bool explicitContext__)
        {
            if(explicitContext__ && context__ == null)
            {
                context__ = emptyContext_;
            }

            InvocationObserver observer__ = IceInternal.ObserverHelper.get(this, __ice_id_name, context__);
            int cnt__ = 0;
            try
            {
                while(true)
                {
                    ObjectDel_ del__ = null;
                    try
                    {
                        checkTwowayOnly__(__ice_id_name);
                        del__ = getDelegate__(false);
                        return del__.ice_id(context__, observer__);
                    }
                    catch(IceInternal.LocalExceptionWrapper ex__)
                    {
                        handleExceptionWrapperRelaxed__(del__, ex__, true, ref cnt__, observer__);
                    }
                    catch(LocalException ex__)
                    {
                        handleException__(del__, ex__, true, ref cnt__, observer__);
                    }
                }
            }
            finally
            {
                if(observer__ != null)
                {
                    observer__.detach();
                }
            }
        }

        public AsyncResult<Callback_Object_ice_id> begin_ice_id()
        {
            return begin_ice_id(null, false, null, null);
        }

        public AsyncResult<Callback_Object_ice_id> begin_ice_id(Dictionary<string, string> context__)
        {
            return begin_ice_id(context__, true, null, null);
        }

        public AsyncResult begin_ice_id(AsyncCallback cb__, object cookie__)
        {
            return begin_ice_id(null, false, cb__, cookie__);
        }

        public AsyncResult begin_ice_id(Dictionary<string, string> context__, AsyncCallback cb__, object cookie__)
        {
            return begin_ice_id(null, false, cb__, cookie__);
        }

        internal const string __ice_id_name = "ice_id";

        public string end_ice_id(AsyncResult r__)
        {
            IceInternal.OutgoingAsync outAsync__ = (IceInternal.OutgoingAsync)r__;
            IceInternal.OutgoingAsync.check__(outAsync__, this, __ice_id_name);
            bool ok = outAsync__.wait__();
            try
            {
                if(!ok)
                {
                    try
                    {
                        outAsync__.throwUserException__();
                    }
                    catch(Ice.UserException ex__)
                    {
                        throw new Ice.UnknownUserException(ex__.ice_name(), ex__);
                    }
                }
                string ret__;
                IceInternal.BasicStream is__ = outAsync__.startReadParams__();
                ret__ = is__.readString();
                outAsync__.endReadParams__();
                return ret__;
            }
            catch(Ice.LocalException ex)
            {
                InvocationObserver obsv__ = outAsync__.getObserver__();
                if(obsv__ != null)
                {
                    obsv__.failed(ex.ice_name());
                }
                throw ex;
            }
        }

        private AsyncResult<Callback_Object_ice_id> begin_ice_id(Dictionary<string, string> context__,
                                                                    bool explicitContext__,
                                                                    Ice.AsyncCallback cb__,
                                                                    object cookie__)
        {
            IceInternal.TwowayOutgoingAsync<Callback_Object_ice_id> result__ =
                new IceInternal.TwowayOutgoingAsync<Callback_Object_ice_id>(this, __ice_id_name, ice_id_completed__,
                                                                            cookie__);
            if(cb__ != null)
            {
                result__.whenCompletedWithAsyncCallback(cb__);
            }
            checkAsyncTwowayOnly__(__ice_id_name);

            try
            {
                result__.prepare__(__ice_id_name, OperationMode.Nonmutating, context__, explicitContext__);
                result__.writeEmptyParams__();
                result__.send__(true);
            }
            catch(Ice.LocalException ex__)
            {
                result__.exceptionAsync__(ex__);
            }
            return result__;
        }

        private void ice_id_completed__(AsyncResult r__, Callback_Object_ice_id cb__, Ice.ExceptionCallback excb__)
        {
            string ret__;
            try
            {
                ret__ = end_ice_id(r__);
            }
            catch(Ice.Exception ex__)
            {
                if(excb__ != null)
                {
                    excb__(ex__);
                }
                return;
            }
            if(cb__ != null)
            {
                cb__(ret__);
            }
        }

        /// <summary>
        /// Invokes an operation dynamically.
        /// </summary>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <param name="outEncaps">The encoded out-paramaters and return value
        /// for the operation. The return value follows any out-parameters.</param>
        /// <returns>If the operation completed successfully, the return value
        /// is true. If the operation raises a user exception,
        /// the return value is false; in this case, outEncaps
        /// contains the encoded user exception. If the operation raises a run-time exception,
        /// it throws it directly.</returns>
        public bool ice_invoke(string operation, OperationMode mode, byte[] inEncaps, out byte[] outEncaps)
        {
            return ice_invoke(operation, mode, inEncaps, out outEncaps, null, false);
        }

        /// <summary>
        /// Invokes an operation dynamically.
        /// </summary>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <param name="outEncaps">The encoded out-paramaters and return value
        /// for the operation. The return value follows any out-parameters.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>If the operation completed successfully, the return value
        /// is true. If the operation raises a user exception,
        /// the return value is false; in this case, outEncaps
        /// contains the encoded user exception. If the operation raises a run-time exception,
        /// it throws it directly.</returns>
        public bool ice_invoke(string operation, OperationMode mode, byte[] inEncaps, out byte[] outEncaps,
                               Dictionary<string, string> context)
        {
            return ice_invoke(operation, mode, inEncaps, out outEncaps, context, true);
        }

        private bool ice_invoke(string operation, OperationMode mode, byte[] inEncaps, out byte[] outEncaps,
                                Dictionary<string, string> context,  bool explicitContext)
        {
            if(explicitContext && context == null)
            {
                context = emptyContext_;
            }

            InvocationObserver observer = IceInternal.ObserverHelper.get(this, operation, context);
            int cnt__ = 0;
            try
            {
                while(true)
                {
                    ObjectDel_ del__ = null;
                    try
                    {
                        del__ = getDelegate__(false);
                        return del__.ice_invoke(operation, mode, inEncaps, out outEncaps, context, observer);
                    }
                    catch(IceInternal.LocalExceptionWrapper ex__)
                    {
                        if(mode == OperationMode.Nonmutating || mode == OperationMode.Idempotent)
                        {
                            handleExceptionWrapperRelaxed__(del__, ex__, true, ref cnt__, observer);
                        }
                        else
                        {
                            handleExceptionWrapper__(del__, ex__, observer);
                        }
                    }
                    catch(LocalException ex__)
                    {
                        handleException__(del__, ex__, true, ref cnt__, observer);
                    }
                }
            }
            finally
            {
                if(observer != null)
                {
                    observer.detach();
                }
            }
        }

        /// <summary>
        /// Invokes an operation dynamically and asynchronously.
        /// </summary>
        /// <param name="cb">The callback object to notify when the operation completes.</param>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <returns> If the operation was invoked synchronously (because there
        /// was no need to queue the request), the return value is true;
        /// otherwise, if the invocation was queued, the return value is false.</returns>
        public bool ice_invoke_async(AMI_Object_ice_invoke cb, string operation, OperationMode mode, byte[] inEncaps)
        {
            AsyncResult<Callback_Object_ice_invoke> result = begin_ice_invoke(operation, mode, inEncaps);
            result.whenCompleted(cb.response__, cb.exception__);
            if(cb is Ice.AMISentCallback)
            {
                result.whenSent((Ice.AsyncCallback)cb.sent__);
            }
            return result.sentSynchronously();
        }

        /// <summary>
        /// Invokes an operation dynamically and asynchronously.
        /// </summary>
        /// <param name="cb">The callback object to notify when the operation completes.</param>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns> If the operation was invoked synchronously (because there
        /// was no need to queue the request), the return value is true;
        /// otherwise, if the invocation was queued, the return value is false.</returns>
        public bool ice_invoke_async(AMI_Object_ice_invoke cb, string operation, OperationMode mode, byte[] inEncaps,
                                     Dictionary<string, string> context)
        {
            AsyncResult<Callback_Object_ice_invoke> result = begin_ice_invoke(operation, mode, inEncaps, context);
            result.whenCompleted(cb.response__, cb.exception__);
            if(cb is Ice.AMISentCallback)
            {
                result.whenSent((Ice.AsyncCallback)cb.sent__);
            }
            return result.sentSynchronously();
        }

        public AsyncResult<Callback_Object_ice_invoke> begin_ice_invoke(string operation,
                                                                           OperationMode mode,
                                                                           byte[] inEncaps)
        {
            return begin_ice_invoke(operation, mode, inEncaps, null, false, null, null);
        }

        public AsyncResult<Callback_Object_ice_invoke> begin_ice_invoke(string operation,
                                                                           OperationMode mode,
                                                                           byte[] inEncaps,
                                                                           Dictionary<string, string> context__)
        {
            return begin_ice_invoke(operation, mode, inEncaps, context__, true, null, null);
        }

        public AsyncResult begin_ice_invoke(string operation, OperationMode mode, byte[] inEncaps, AsyncCallback cb__,
                                            object cookie__)
        {
            return begin_ice_invoke(operation, mode, inEncaps, null, false, cb__, cookie__);
        }

        public AsyncResult begin_ice_invoke(string operation, OperationMode mode, byte[] inEncaps,
                                            Dictionary<string, string> context__, AsyncCallback cb__, object cookie__)
        {
            return begin_ice_invoke(operation, mode, inEncaps, null, false, cb__, cookie__);
        }

        internal const string __ice_invoke_name = "ice_invoke";

        public bool end_ice_invoke(out byte[] outEncaps, AsyncResult r__)
        {
            IceInternal.OutgoingAsync outAsync__ = (IceInternal.OutgoingAsync)r__;
            IceInternal.OutgoingAsync.check__(outAsync__, this, __ice_invoke_name);
            bool ok = outAsync__.wait__();
            try
            {
                if(_reference.getMode() == IceInternal.Reference.Mode.ModeTwoway)
                {
                    outEncaps = outAsync__.readParamEncaps__();
                }
                else
                {
                    outEncaps = null; // Satisfy compiler
                }
                return ok;
            }
            catch(Ice.LocalException ex)
            {
                InvocationObserver obsv__ = outAsync__.getObserver__();
                if(obsv__ != null)
                {
                    obsv__.failed(ex.ice_name());
                }
                throw ex;
            }
        }

        private AsyncResult<Callback_Object_ice_invoke> begin_ice_invoke(string operation,
                                                                         OperationMode mode,
                                                                         byte[] inEncaps,
                                                                         Dictionary<string, string> context__,
                                                                         bool explicitContext__,
                                                                         Ice.AsyncCallback cb__,
                                                                         object cookie__)
        {
            IceInternal.TwowayOutgoingAsync<Callback_Object_ice_invoke> result__ =
                new IceInternal.TwowayOutgoingAsync<Callback_Object_ice_invoke>(this, __ice_invoke_name,
                                                                                ice_invoke_completed__, cookie__);
            if(cb__ != null)
            {
                result__.whenCompletedWithAsyncCallback(cb__);
            }

            try
            {
                result__.prepare__(operation, mode, context__, explicitContext__);
                result__.writeParamEncaps__(inEncaps);
                result__.send__(true);
            }
            catch(Ice.LocalException ex__)
            {
                result__.exceptionAsync__(ex__);
            }
            return result__;
        }

        private void ice_invoke_completed__(AsyncResult r__,
                                            Callback_Object_ice_invoke cb__,
                                            Ice.ExceptionCallback excb__)
        {
            byte[] outEncaps;
            bool ret__;
            try
            {
                ret__ = end_ice_invoke(out outEncaps, r__);
            }
            catch(Ice.Exception ex__)
            {
                if(excb__ != null)
                {
                    excb__(ex__);
                }
                return;
            }
            if(cb__ != null)
            {
                cb__(ret__, outEncaps);
            }
        }

        /// <summary>
        /// Returns the identity embedded in this proxy.
        /// <returns>The identity of the target object.</returns>
        /// </summary>
        public Identity ice_getIdentity()
        {
            return (Identity)_reference.getIdentity().Clone();
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the per-proxy context.
        /// <param name="newIdentity">The identity for the new proxy.</param>
        /// <returns>The proxy with the new identity.</returns>
        /// </summary>
        public ObjectPrx ice_identity(Identity newIdentity)
        {
            if(newIdentity.name.Length == 0)
            {
                throw new IllegalIdentityException();
            }
            if(newIdentity.Equals(_reference.getIdentity()))
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

        /// <summary>
        /// Returns the per-proxy context for this proxy.
        /// </summary>
        /// <returns>The per-proxy context. If the proxy does not have a per-proxy (implicit) context, the return value
        /// is null.</returns>
        public Dictionary<string, string> ice_getContext()
        {
            return new Dictionary<string, string>(_reference.getContext());
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the per-proxy context.
        /// </summary>
        /// <param name="newContext">The context for the new proxy.</param>
        /// <returns>The proxy with the new per-proxy context.</returns>
        public ObjectPrx ice_context(Dictionary<string, string> newContext)
        {
            return newInstance(_reference.changeContext(newContext));
        }

        /// <summary>
        /// Returns the facet for this proxy.
        /// </summary>
        /// <returns>The facet for this proxy. If the proxy uses the default facet, the return value is the
        /// empty string.</returns>
        public string ice_getFacet()
        {
            return _reference.getFacet();
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the facet.
        /// </summary>
        /// <param name="newFacet">The facet for the new proxy.</param>
        /// <returns>The proxy with the new facet.</returns>
        public ObjectPrx ice_facet(string newFacet)
        {
            if(newFacet == null)
            {
                newFacet = "";
            }

            if(newFacet.Equals(_reference.getFacet()))
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

        /// <summary>
        /// Returns the adapter ID for this proxy.
        /// </summary>
        /// <returns>The adapter ID. If the proxy does not have an adapter ID, the return value is the
        /// empty string.</returns>
        public string ice_getAdapterId()
        {
            return _reference.getAdapterId();
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the adapter ID.
        /// </summary>
        /// <param name="newAdapterId">The adapter ID for the new proxy.</param>
        /// <returns>The proxy with the new adapter ID.</returns>
        public ObjectPrx ice_adapterId(string newAdapterId)
        {
            if(newAdapterId == null)
            {
                newAdapterId = "";
            }

            if(newAdapterId.Equals(_reference.getAdapterId()))
            {
                return this;
            }
            else
            {
                return newInstance(_reference.changeAdapterId(newAdapterId));
            }
        }

        /// <summary>
        /// Returns the endpoints used by this proxy.
        /// </summary>
        /// <returns>The endpoints used by this proxy.</returns>
        public Endpoint[] ice_getEndpoints()
        {
            return (Endpoint[])_reference.getEndpoints().Clone();
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the endpoints.
        /// </summary>
        /// <param name="newEndpoints">The endpoints for the new proxy.</param>
        /// <returns>The proxy with the new endpoints.</returns>
        public ObjectPrx ice_endpoints(Endpoint[] newEndpoints)
        {
            if(Arrays.Equals(newEndpoints, _reference.getEndpoints()))
            {
                return this;
            }
            else
            {
                IceInternal.EndpointI[] endpts = new IceInternal.EndpointI[newEndpoints.Length];
                for(int i = 0; i < newEndpoints.Length; ++i)
                {
                    endpts[i] = (IceInternal.EndpointI)newEndpoints[i];
                }
                return newInstance(_reference.changeEndpoints(endpts));
            }
        }

        /// <summary>
        /// Returns the locator cache timeout of this proxy.
        /// </summary>
        /// <returns>The locator cache timeout value (in seconds).</returns>
        public int ice_getLocatorCacheTimeout()
        {
            return _reference.getLocatorCacheTimeout();
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the locator cache timeout.
        /// </summary>
        /// <param name="newTimeout">The new locator cache timeout (in seconds).</param>
        public ObjectPrx ice_locatorCacheTimeout(int newTimeout)
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

        /// <summary>
        /// Returns whether this proxy caches connections.
        /// </summary>
        /// <returns>True if this proxy caches connections; false, otherwise.</returns>
        public bool ice_isConnectionCached()
        {
            return _reference.getCacheConnection();
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for connection caching.
        /// </summary>
        /// <param name="newCache">True if the new proxy should cache connections; false, otherwise.</param>
        /// <returns>The new proxy with the specified caching policy.</returns>
        public ObjectPrx ice_connectionCached(bool newCache)
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

        /// <summary>
        /// Returns how this proxy selects endpoints (randomly or ordered).
        /// </summary>
        /// <returns>The endpoint selection policy.</returns>
        public EndpointSelectionType ice_getEndpointSelection()
        {
            return _reference.getEndpointSelection();
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the endpoint selection policy.
        /// </summary>
        /// <param name="newType">The new endpoint selection policy.</param>
        /// <returns>The new proxy with the specified endpoint selection policy.</returns>
        public ObjectPrx ice_endpointSelection(EndpointSelectionType newType)
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

        /// <summary>
        /// Returns whether this proxy communicates only via secure endpoints.
        /// </summary>
        /// <returns>True if this proxy communicates only vi secure endpoints; false, otherwise.</returns>
        public bool ice_isSecure()
        {
            return _reference.getSecure();
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for how it selects endpoints.
        /// </summary>
        /// <param name="b"> If b is true, only endpoints that use a secure transport are
        /// used by the new proxy. If b is false, the returned proxy uses both secure and insecure
        /// endpoints.</param>
        /// <returns>The new proxy with the specified selection policy.</returns>
        public ObjectPrx ice_secure(bool b)
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

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the encoding used to marshal
        /// parameters.
        /// </summary>
        /// <param name="e">The encoding version to use to marshal requests parameters.</param>
        /// <returns>The new proxy with the specified encoding version.</returns>
        public ObjectPrx ice_encodingVersion(Ice.EncodingVersion e)
        {
            if(e.Equals(_reference.getEncoding()))
            {
                return this;
            }
            else
            {
                return newInstance(_reference.changeEncoding(e));
            }
        }

        /// <summary>Returns the encoding version used to marshal requests parameters.</summary>
        /// <returns>The encoding version.</returns>
        public Ice.EncodingVersion ice_getEncodingVersion()
        {
            return _reference.getEncoding();
        }

        /// <summary>
        /// Returns whether this proxy prefers secure endpoints.
        /// </summary>
        /// <returns>True if the proxy always attempts to invoke via secure endpoints before it
        /// attempts to use insecure endpoints; false, otherwise.</returns>
        public bool ice_isPreferSecure()
        {
            return _reference.getPreferSecure();
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for its endpoint selection policy.
        /// </summary>
        /// <param name="b">If b is true, the new proxy will use secure endpoints for invocations
        /// and only use insecure endpoints if an invocation cannot be made via secure endpoints. If b is
        /// false, the proxy prefers insecure endpoints to secure ones.</param>
        /// <returns>The new proxy with the new endpoint selection policy.</returns>
        public ObjectPrx ice_preferSecure(bool b)
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

        /// <summary>
        /// Returns the router for this proxy.
        /// </summary>
        /// <returns>The router for the proxy. If no router is configured for the proxy, the return value
        /// is null.</returns>
        public Ice.RouterPrx ice_getRouter()
        {
            IceInternal.RouterInfo ri = _reference.getRouterInfo();
            return ri != null ? ri.getRouter() : null;
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the router.
        /// </summary>
        /// <param name="router">The router for the new proxy.</param>
        /// <returns>The new proxy with the specified router.</returns>
        public ObjectPrx ice_router(RouterPrx router)
        {
            IceInternal.Reference @ref = _reference.changeRouter(router);
            if(@ref.Equals(_reference))
            {
                return this;
            }
            else
            {
                return newInstance(@ref);
            }
        }

        /// <summary>
        /// Returns the locator for this proxy.
        /// </summary>
        /// <returns>The locator for this proxy. If no locator is configured, the return value is null.</returns>
        public Ice.LocatorPrx ice_getLocator()
        {
            IceInternal.LocatorInfo li = _reference.getLocatorInfo();
            return li != null ? li.getLocator() : null;
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the locator.
        /// </summary>
        /// <param name="locator">The locator for the new proxy.</param>
        /// <returns>The new proxy with the specified locator.</returns>
        public ObjectPrx ice_locator(LocatorPrx locator)
        {
            IceInternal.Reference @ref = _reference.changeLocator(locator);
            if(@ref.Equals(_reference))
            {
                return this;
            }
            else
            {
                return newInstance(@ref);
            }
        }

        /// <summary>
        /// Returns whether this proxy uses collocation optimization.
        /// </summary>
        /// <returns>True if the proxy uses collocation optimization; false, otherwise.</returns>
        public bool ice_isCollocationOptimized()
        {
            return _reference.getCollocationOptimized();
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for collocation optimization.
        /// </summary>
        /// <param name="b">True if the new proxy enables collocation optimization; false, otherwise.</param>
        /// <returns>The new proxy the specified collocation optimization.</returns>
        public ObjectPrx ice_collocationOptimized(bool b)
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

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, but uses twoway invocations.
        /// </summary>
        /// <returns>A new proxy that uses twoway invocations.</returns>
        public ObjectPrx ice_twoway()
        {
            if(_reference.getMode() == IceInternal.Reference.Mode.ModeTwoway)
            {
                return this;
            }
            else
            {
                return newInstance(_reference.changeMode(IceInternal.Reference.Mode.ModeTwoway));
            }
        }

        /// <summary>
        /// Returns whether this proxy uses twoway invocations.
        /// </summary>
        /// <returns>True if this proxy uses twoway invocations; false, otherwise.</returns>
        public bool ice_isTwoway()
        {
            return _reference.getMode() == IceInternal.Reference.Mode.ModeTwoway;
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, but uses oneway invocations.
        /// </summary>
        /// <returns>A new proxy that uses oneway invocations.</returns>
        public ObjectPrx ice_oneway()
        {
            if(_reference.getMode() == IceInternal.Reference.Mode.ModeOneway)
            {
                return this;
            }
            else
            {
                return newInstance(_reference.changeMode(IceInternal.Reference.Mode.ModeOneway));
            }
        }

        /// <summary>
        /// Returns whether this proxy uses oneway invocations.
        /// </summary>
        /// <returns>True if this proxy uses oneway invocations; false, otherwise.</returns>
        public bool ice_isOneway()
        {
            return _reference.getMode() == IceInternal.Reference.Mode.ModeOneway;
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, but uses batch oneway invocations.
        /// </summary>
        /// <returns>A new proxy that uses batch oneway invocations.</returns>
        public ObjectPrx ice_batchOneway()
        {
            if(_reference.getMode() == IceInternal.Reference.Mode.ModeBatchOneway)
            {
                return this;
            }
            else
            {
                return newInstance(_reference.changeMode(IceInternal.Reference.Mode.ModeBatchOneway));
            }
        }

        /// <summary>
        /// Returns whether this proxy uses batch oneway invocations.
        /// </summary>
        /// <returns>True if this proxy uses batch oneway invocations; false, otherwise.</returns>
        public bool ice_isBatchOneway()
        {
            return _reference.getMode() == IceInternal.Reference.Mode.ModeBatchOneway;
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, but uses datagram invocations.
        /// </summary>
        /// <returns>A new proxy that uses datagram invocations.</returns>
        public ObjectPrx ice_datagram()
        {
            if(_reference.getMode() == IceInternal.Reference.Mode.ModeDatagram)
            {
                return this;
            }
            else
            {
                return newInstance(_reference.changeMode(IceInternal.Reference.Mode.ModeDatagram));
            }
        }

        /// <summary>
        /// Returns whether this proxy uses datagram invocations.
        /// </summary>
        /// <returns>True if this proxy uses datagram invocations; false, otherwise.</returns>
        public bool ice_isDatagram()
        {
            return _reference.getMode() == IceInternal.Reference.Mode.ModeDatagram;
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, but uses batch datagram invocations.
        /// </summary>
        /// <returns>A new proxy that uses batch datagram invocations.</returns>
        public ObjectPrx ice_batchDatagram()
        {
            if(_reference.getMode() == IceInternal.Reference.Mode.ModeBatchDatagram)
            {
                return this;
            }
            else
            {
                return newInstance(_reference.changeMode(IceInternal.Reference.Mode.ModeBatchDatagram));
            }
        }

        /// <summary>
        /// Returns whether this proxy uses batch datagram invocations.
        /// </summary>
        /// <returns>True if this proxy uses batch datagram invocations; false, otherwise.</returns>
        public bool ice_isBatchDatagram()
        {
            return _reference.getMode() == IceInternal.Reference.Mode.ModeBatchDatagram;
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for compression.
        /// </summary>
        /// <param name="co">True enables compression for the new proxy; false disables compression.</param>
        /// <returns>A new proxy with the specified compression setting.</returns>
        public ObjectPrx ice_compress(bool co)
        {
            IceInternal.Reference @ref = _reference.changeCompress(co);
            if(@ref.Equals(_reference))
            {
                return this;
            }
            else
            {
                return newInstance(@ref);
            }
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for its timeout setting.
        /// </summary>
        /// <param name="t">The timeout for the new proxy in milliseconds.</param>
        /// <returns>A new proxy with the specified timeout.</returns>
        public ObjectPrx ice_timeout(int t)
        {
            IceInternal.Reference @ref = _reference.changeTimeout(t);
            if(@ref.Equals(_reference))
            {
                return this;
            }
            else
            {
                return newInstance(@ref);
            }
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for its connection ID.
        /// </summary>
        /// <param name="connectionId">The connection ID for the new proxy. An empty string removes the
        /// connection ID.</param>
        /// <returns>A new proxy with the specified connection ID.</returns>
        public ObjectPrx ice_connectionId(string connectionId)
        {
            IceInternal.Reference @ref = _reference.changeConnectionId(connectionId);
            if(@ref.Equals(_reference))
            {
                return this;
            }
            else
            {
                return newInstance(@ref);
            }
        }

        /// <summary>
        /// Returns the connection id of this proxy.
        /// </summary>
        /// <returns>The connection id.</returns>
        public string ice_getConnectionId()
        {
            return _reference.getConnectionId();
        }

        /// <summary>
        /// Returns the Connection for this proxy. If the proxy does not yet have an established connection,
        /// it first attempts to create a connection.
        /// </summary>
        /// <returns>The Connection for this proxy.</returns>
        /// <exception name="CollocationOptimizationException">If the proxy uses collocation optimization and denotes a
        /// collocated object.</exception>
        public Connection ice_getConnection()
        {
            InvocationObserver observer = IceInternal.ObserverHelper.get(this, "ice_getConnection");
            int cnt__ = 0;
            try
            {
                while(true)
                {
                    ObjectDel_ del__ = null;
                    try
                    {
                        del__ = getDelegate__(false);
                        // Wait for the connection to be established.
                        return del__.getRequestHandler__().getConnection(true);
                    }
                    catch(LocalException ex__)
                    {
                        handleException__(del__, ex__, true, ref cnt__, observer);
                    }
                }
            }
            finally
            {
                if(observer != null)
                {
                    observer.detach();
                }
            }
        }

        /// <summary>
        /// Returns the cached Connection for this proxy. If the proxy does not yet have an established
        /// connection, it does not attempt to create a connection.
        /// </summary>
        /// <returns>The cached Connection for this proxy (null if the proxy does not have
        /// an established connection).</returns>
        /// <exception name="CollocationOptimizationException">If the proxy uses collocation optimization and denotes a
        /// collocated object.</exception>
        public Connection ice_getCachedConnection()
        {
            ObjectDel_ del__ = null;
            lock(this)
            {
                del__ = _delegate;
            }

            if(del__ != null)
            {
                try
                {
                    // Wait for the connection to be established.
                    return del__.getRequestHandler__().getConnection(false);
                }
                catch(LocalException)
                {
                }
            }
            return null;
        }

        /// <summary>
        /// Flushes any pending batched requests for this communicator. The call blocks until the flush is complete.
        /// </summary>
        public void ice_flushBatchRequests()
        {
            //
            // We don't automatically retry if ice_flushBatchRequests fails. Otherwise, if some batch
            // requests were queued with the connection, they would be lost without being noticed.
            //
            ObjectDel_ del__ = null;
            InvocationObserver observer = 
                IceInternal.ObserverHelper.get(this, ObjectPrxHelperBase.__ice_flushBatchRequests_name);
            int cnt__ = -1; // Don't retry.
            try
            {
                del__ = getDelegate__(false);
                del__.ice_flushBatchRequests(observer);
                return;
            }
            catch(LocalException ex__)
            {
                handleException__(del__, ex__, true, ref cnt__, observer);
            }
            finally
            {
                if(observer != null)
                {
                    observer.detach();
                }
            }
        }

        /// <summary>
        /// Asynchronously flushes any pending batched requests for this communicator. The call does not block.
        /// </summary>
        /// <param name="cb">The callback object to notify the application when the flush is complete.</param>
        /// <returns>True if the requests were flushed immediately without blocking; false
        /// if the requests could not be flushed immediately.</returns>
        public bool ice_flushBatchRequests_async(AMI_Object_ice_flushBatchRequests cb)
        {
            Ice.AsyncResult result = begin_ice_flushBatchRequests().whenCompleted(cb.exception__);
            if(cb is Ice.AMISentCallback)
            {
                result.whenSent((Ice.AsyncCallback)cb.sent__);
            }
            return result.sentSynchronously();
        }

        internal const string __ice_flushBatchRequests_name = "ice_flushBatchRequests";

        public AsyncResult begin_ice_flushBatchRequests()
        {
            return begin_ice_flushBatchRequests(null, null);
        }

        public AsyncResult begin_ice_flushBatchRequests(Ice.AsyncCallback cb__, object cookie__)
        {
            IceInternal.ProxyBatchOutgoingAsync result__ =
                new IceInternal.ProxyBatchOutgoingAsync(this, __ice_flushBatchRequests_name, cookie__);
            if(cb__ != null)
            {
                result__.whenCompletedWithAsyncCallback(cb__);
            }
            try
            {
                result__.send__();
            }
            catch(Ice.LocalException ex__)
            {
                result__.exceptionAsync__(ex__);
            }
            return result__;
        }

        public void end_ice_flushBatchRequests(Ice.AsyncResult r__)
        {
            IceInternal.BatchOutgoingAsync outAsync__ = (IceInternal.BatchOutgoingAsync)r__;
            IceInternal.BatchOutgoingAsync.check__(outAsync__, this, __ice_flushBatchRequests_name);
            outAsync__.wait__();
        }

        /// <summary>
        /// Returns whether this proxy equals the passed object. Two proxies are equal if they are equal in all
        /// respects, that is, if their object identity, endpoints timeout settings, and so on are all equal.
        /// </summary>
        /// <param name="r">The object to compare this proxy with.</param>
        /// <returns>True if this proxy is equal to r; false, otherwise.</returns>
        public override bool Equals(object r)
        {
            ObjectPrxHelperBase rhs = r as ObjectPrxHelperBase;
            return object.ReferenceEquals(rhs, null) ? false : _reference.Equals(rhs._reference);
        }

        /// <summary>
        /// Returns whether two proxies are equal. Two proxies are equal if they are equal in all
        /// respects, that is, if their object identity, endpoints timeout settings, and so on are all equal.
        /// </summary>
        /// <param name="lhs">A proxy to compare with the proxy rhs.</param>
        /// <param name="rhs">A proxy to compare with the proxy lhs.</param>
        /// <returns>True if the proxies are equal; false, otherwise.</returns>
        public static bool Equals(ObjectPrxHelperBase lhs, ObjectPrxHelperBase rhs)
        {
            return object.ReferenceEquals(lhs, null) ? object.ReferenceEquals(rhs, null) : lhs.Equals(rhs);
        }

        /// <summary>
        /// Returns whether two proxies are equal. Two proxies are equal if they are equal in all
        /// respects, that is, if their object identity, endpoints timeout settings, and so on are all equal.
        /// </summary>
        /// <param name="lhs">A proxy to compare with the proxy rhs.</param>
        /// <param name="rhs">A proxy to compare with the proxy lhs.</param>
        /// <returns>True if the proxies are equal; false, otherwise.</returns>
        public static bool operator==(ObjectPrxHelperBase lhs, ObjectPrxHelperBase rhs)
        {
            return Equals(lhs, rhs);
        }

        /// <summary>
        /// Returns whether two proxies are not equal. Two proxies are equal if they are equal in all
        /// respects, that is, if their object identity, endpoints timeout settings, and so on are all equal.
        /// </summary>
        /// <param name="lhs">A proxy to compare with the proxy rhs.</param>
        /// <param name="rhs">A proxy to compare with the proxy lhs.</param>
        /// <returns>True if the proxies are not equal; false, otherwise.</returns>
        public static bool operator!=(ObjectPrxHelperBase lhs, ObjectPrxHelperBase rhs)
        {
            return !Equals(lhs, rhs);
        }

        public IceInternal.Reference reference__()
        {
            return _reference;
        }

        public void copyFrom__(ObjectPrx from)
        {
            ObjectPrxHelperBase h = (ObjectPrxHelperBase)from;
            IceInternal.Reference @ref = null;
            ObjectDelM_ delegateM = null;
            ObjectDelD_ delegateD = null;

            lock(from)
            {
                @ref = h._reference;
                delegateM = h._delegate as ObjectDelM_;
                delegateD = h._delegate as ObjectDelD_;
            }

            //
            // No need to synchronize "*this", as this operation is only
            // called upon initialization.
            //

            Debug.Assert(_reference == null);
            Debug.Assert(_delegate == null);

            _reference = @ref;

            if(_reference.getCacheConnection())
            {
                //
                // The _delegate attribute is only used if "cache connection"
                // is enabled. If it's not enabled, we don't keep track of the
                // delegate -- a new delegate is created for each invocations.
                //

                if(delegateD != null)
                {
                    ObjectDelD_ @delegate = createDelegateD__();
                    @delegate.copyFrom__(delegateD);
                    _delegate = @delegate;
                }
                else if(delegateM != null)
                {
                    ObjectDelM_ @delegate = createDelegateM__();
                    @delegate.copyFrom__(delegateM);
                    _delegate = @delegate;
                }
            }
        }

        public int handleException__(ObjectDel_ @delegate, LocalException ex, bool sleep, ref int cnt, 
                                     InvocationObserver obsv)
        {
            //
            // Only _delegate needs to be mutex protected here.
            //
            lock(this)
            {
                if(@delegate == _delegate)
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
                
                int interval;
                try
                {
                    interval = _reference.getInstance().proxyFactory().checkRetryAfterException(ex, _reference, sleep,
                                                                                                ref cnt);
                }
                catch(CommunicatorDestroyedException)
                {
                    //
                    // The communicator is already destroyed, so we cannot
                    // retry.
                    //
                    throw ex;
                }
                if(obsv != null)
                {
                    obsv.retried();
                }
                return interval;
            }
            catch(Ice.LocalException e)
            {
                if(obsv != null)
                {
                    obsv.failed(e.ice_name());
                }
                throw;
            }
        }

        public int handleExceptionWrapper__(ObjectDel_ @delegate, IceInternal.LocalExceptionWrapper ex,
                                            InvocationObserver obsv)
        {
            lock(this)
            {
                if(@delegate == _delegate)
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

            return 0;
        }

        public int handleExceptionWrapperRelaxed__(ObjectDel_ @delegate, IceInternal.LocalExceptionWrapper ex,
                                                   bool sleep, ref int cnt, InvocationObserver obsv)
        {
            if(!ex.retry())
            {
                return handleException__(@delegate, ex.get(), sleep, ref cnt, obsv);
            }
            else
            {
                lock(this)
                {
                    if(@delegate == _delegate)
                    {
                        _delegate = null;
                    }
                }
                return 0;
            }
        }

        public void checkTwowayOnly__(string name)
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

        public void checkAsyncTwowayOnly__(string name)
        {
            //
            // No mutex lock necessary, there is nothing mutable in this
            // operation.
            //

            if(!ice_isTwoway())
            {
                throw new System.ArgumentException("`" + name + "' can only be called with a twoway proxy");
            }
        }

        public void end__(AsyncResult result, string operation)
        {
            IceInternal.OutgoingAsync outAsync = (IceInternal.OutgoingAsync)result;
            IceInternal.OutgoingAsync.check__(outAsync, this, operation);
            bool ok = outAsync.wait__();
            if(_reference.getMode() == IceInternal.Reference.Mode.ModeTwoway)
            {
                try
                {
                    if(!ok)
                    {
                        try
                        {
                            outAsync.throwUserException__();
                        }
                        catch(Ice.UserException ex)
                        {
                            throw new Ice.UnknownUserException(ex.ice_name(), ex);
                        }
                    }
                    outAsync.readEmptyParams__();
                }
                catch(Ice.LocalException ex)
                {
                    InvocationObserver obsv__ = outAsync.getObserver__();
                    if(obsv__ != null)
                    {
                        obsv__.failed(ex.ice_name());
                    }
                    throw ex;
                }
            }
        }

        public ObjectDel_ getDelegate__(bool ami)
        {
            if(_reference.getCacheConnection())
            {
                lock(this)
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
                IceInternal.Reference.Mode mode = _reference.getMode();
                return createDelegate(ami ||
                                      mode == IceInternal.Reference.Mode.ModeBatchOneway ||
                                      mode == IceInternal.Reference.Mode.ModeBatchDatagram);
            }
        }

        public void setRequestHandler__(ObjectDel_ @delegate, IceInternal.RequestHandler handler)
        {
            lock(this)
            {
                if(@delegate == _delegate)
                {
                    if(_delegate is ObjectDelM_)
                    {
                        _delegate = createDelegateM__();
                        _delegate.setRequestHandler__(handler);
                    }
                    else if(_delegate is ObjectDelD_)
                    {
                        _delegate = createDelegateD__();
                        _delegate.setRequestHandler__(handler);
                    }
                }
            }
        }

        protected virtual ObjectDelM_ createDelegateM__()
        {
            return new ObjectDelM_();
        }

        protected virtual ObjectDelD_ createDelegateD__()
        {
            return new ObjectDelD_();
        }

        private ObjectDel_ createDelegate(bool async)
        {
            if(_reference.getCollocationOptimized())
            {
                ObjectAdapter adapter = _reference.getInstance().objectAdapterFactory().findObjectAdapter(this);
                if(adapter != null)
                {
                    ObjectDelD_ d = createDelegateD__();
                    d.setup(_reference, adapter);
                    return d;
                }
            }

            ObjectDelM_ d2 = createDelegateM__();
            d2.setup(_reference, this, async);
            return d2;
        }

        //
        // Only for use by IceInternal.ProxyFactory
        //
        public void setup(IceInternal.Reference @ref)
        {
            //
            // No need to synchronize, as this operation is only called
            // upon initial initialization.
            //

            Debug.Assert(_reference == null);
            Debug.Assert(_delegate == null);

            _reference = @ref;
        }

        private ObjectPrxHelperBase newInstance(IceInternal.Reference @ref)
        {
            ObjectPrxHelperBase proxy = (ObjectPrxHelperBase)Activator.CreateInstance(GetType());
            proxy.setup(@ref);
            return proxy;
        }

        protected static Dictionary<string, string> emptyContext_ = new Dictionary<string, string>();
        private IceInternal.Reference _reference;
        private ObjectDel_ _delegate;
    }

    /// <summary>
    /// Base class for all proxy helpers.
    /// </summary>
    public class ObjectPrxHelper : ObjectPrxHelperBase
    {
        /// <summary>
        /// Casts a proxy to {@link ObjectPrx}. This call contacts
        /// the server and will throw an Ice run-time exception if the target
        /// object does not exist or the server cannot be reached.
        /// </summary>
        /// <param name="b">The proxy to cast to ObjectPrx.</param>
        /// <returns>b.</returns>
        public static ObjectPrx checkedCast(ObjectPrx b)
        {
            return b;
        }

        /// <summary>
        /// Casts a proxy to {@link ObjectPrx}. This call contacts
        /// the server and throws an Ice run-time exception if the target
        /// object does not exist or the server cannot be reached.
        /// </summary>
        /// <param name="b">The proxy to cast to ObjectPrx.</param>
        /// <param name="ctx">The Context map for the invocation.</param>
        /// <returns>b.</returns>
        public static ObjectPrx checkedCast(ObjectPrx b, Dictionary<string, string> ctx)
        {
            return b;
        }

        /// <summary>
        /// Creates a new proxy that is identical to the passed proxy, except
        /// for its facet. This call contacts
        /// the server and throws an Ice run-time exception if the target
        /// object does not exist, the specified facet does not exist, or the server cannot be reached.
        /// </summary>
        /// <param name="b">The proxy to cast to ObjectPrx.</param>
        /// <param name="f">The facet for the new proxy.</param>
        /// <returns>The new proxy with the specified facet.</returns>
        public static ObjectPrx checkedCast(ObjectPrx b, string f)
        {
            ObjectPrx d = null;
            if(b != null)
            {
                try
                {
                    ObjectPrx bb = b.ice_facet(f);
                    bool ok = bb.ice_isA("::Ice::Object");
                    Debug.Assert(ok);
                    ObjectPrxHelper h = new ObjectPrxHelper();
                    h.copyFrom__(bb);
                    d = h;
                }
                catch(Ice.FacetNotExistException)
                {
                }
            }
            return d;
        }

        /// <summary>
        /// Creates a new proxy that is identical to the passed proxy, except
        /// for its facet. This call contacts
        /// the server and throws an Ice run-time exception if the target
        /// object does not exist, the specified facet does not exist, or the server cannot be reached.
        /// </summary>
        /// <param name="b">The proxy to cast to ObjectPrx.</param>
        /// <param name="f">The facet for the new proxy.</param>
        /// <param name="ctx">The Context map for the invocation.</param>
        /// <returns>The new proxy with the specified facet.</returns>
        public static ObjectPrx checkedCast(ObjectPrx b, string f, Dictionary<string, string> ctx)
        {
            ObjectPrx d = null;
            if(b != null)
            {
                try
                {
                    ObjectPrx bb = b.ice_facet(f);
                    bool ok = bb.ice_isA("::Ice::Object", ctx);
                    Debug.Assert(ok);
                    ObjectPrxHelper h = new ObjectPrxHelper();
                    h.copyFrom__(bb);
                    d = h;
                }
                catch(Ice.FacetNotExistException)
                {
                }
            }
            return d;
        }

        /// <summary>
        /// Casts a proxy to {@link ObjectPrx}. This call does
        /// not contact the server and always succeeds.
        /// </summary>
        /// <param name="b">The proxy to cast to ObjectPrx.</param>
        /// <returns>b.</returns>
        public static ObjectPrx uncheckedCast(ObjectPrx b)
        {
            return b;
        }

        /// <summary>
        /// Creates a new proxy that is identical to the passed proxy, except
        /// for its facet. This call does not contact the server and always succeeds.
        /// </summary>
        /// <param name="b">The proxy to cast to ObjectPrx.</param>
        /// <param name="f">The facet for the new proxy.</param>
        /// <returns>The new proxy with the specified facet.</returns>
        public static ObjectPrx uncheckedCast(ObjectPrx b, string f)
        {
            ObjectPrx d = null;
            if(b != null)
            {
                ObjectPrx bb = b.ice_facet(f);
                ObjectPrxHelper h = new ObjectPrxHelper();
                h.copyFrom__(bb);
                d = h;
            }
            return d;
        }
    }

    public interface ObjectDel_
    {
        bool ice_isA(string id, Dictionary<string, string> context, InvocationObserver obsv);
        void ice_ping(Dictionary<string, string> context, InvocationObserver obsv);
        string[] ice_ids(Dictionary<string, string> context, InvocationObserver obsv);
        string ice_id(Dictionary<string, string> context, InvocationObserver obsv);
        bool ice_invoke(string operation, OperationMode mode, byte[] inEncaps, out byte[] outEncaps,
                        Dictionary<string, string> context, InvocationObserver obsv);

        void ice_flushBatchRequests(InvocationObserver obsv);

        IceInternal.RequestHandler getRequestHandler__();
        void setRequestHandler__(IceInternal.RequestHandler handler);
    }

    public class ObjectDelD_ : ObjectDel_
    {
        public virtual bool ice_isA(string id__, Dictionary<string, string> context__, InvocationObserver obsv__)
        {
            Current current__ = new Current();
            initCurrent__(ref current__, ObjectPrxHelperBase.__ice_isA_name, OperationMode.Nonmutating, context__);

            bool result__ = false;
            IceInternal.Direct.RunDelegate run__ = delegate(Object servant__)
            {
                result__ = servant__.ice_isA(id__, current__);
                return Ice.DispatchStatus.DispatchOK;
            };

            IceInternal.Direct direct__ = null;
            try
            {
                direct__ = new IceInternal.Direct(current__, run__);
            }
            catch(System.Exception ex__)
            {
                IceInternal.LocalExceptionWrapper.throwWrapper(ex__);
            }

            try
            {
                DispatchStatus status__ = direct__.getServant().collocDispatch__(direct__);
                Debug.Assert(status__ == DispatchStatus.DispatchOK);
                return result__;
            }
            finally
            {
                try
                {
                    direct__.destroy();
                }
                catch(System.Exception ex__)
                {
                    IceInternal.LocalExceptionWrapper.throwWrapper(ex__);
                }
            }
        }

        public virtual void ice_ping(Dictionary<string, string> context__, InvocationObserver obsv__)
        {
            Current current__ = new Current();
            initCurrent__(ref current__, ObjectPrxHelperBase.__ice_ping_name, OperationMode.Nonmutating, context__);

            IceInternal.Direct.RunDelegate run__ = delegate(Object servant__)
            {
                servant__.ice_ping(current__);
                return Ice.DispatchStatus.DispatchOK;
            };

            IceInternal.Direct direct__ = null;
            try
            {
                direct__ = new IceInternal.Direct(current__, run__);
            }
            catch(System.Exception ex__)
            {
                IceInternal.LocalExceptionWrapper.throwWrapper(ex__);
            }

            try
            {
                DispatchStatus status__ = direct__.getServant().collocDispatch__(direct__);
                Debug.Assert(status__ == DispatchStatus.DispatchOK);
            }
            finally
            {
                try
                {
                    direct__.destroy();
                }
                catch(System.Exception ex__)
                {
                    IceInternal.LocalExceptionWrapper.throwWrapper(ex__);
                }
            }
        }

        public virtual string[] ice_ids(Dictionary<string, string> context__, InvocationObserver obsv__)
        {
            Current current__ = new Current();
            initCurrent__(ref current__, ObjectPrxHelperBase.__ice_ids_name, OperationMode.Nonmutating, context__);

            string[] result__ = null;
            IceInternal.Direct.RunDelegate run__ = delegate(Object servant__)
            {
                result__ = servant__.ice_ids(current__);
                return Ice.DispatchStatus.DispatchOK;
            };

            IceInternal.Direct direct__ = null;
            try
            {
                direct__ = new IceInternal.Direct(current__, run__);
            }
            catch(System.Exception ex__)
            {
                IceInternal.LocalExceptionWrapper.throwWrapper(ex__);
            }

            try
            {
                DispatchStatus status__ = direct__.getServant().collocDispatch__(direct__);
                Debug.Assert(status__ == DispatchStatus.DispatchOK);
                return result__;
            }
            finally
            {
                try
                {
                    direct__.destroy();
                }
                catch(System.Exception ex__)
                {
                    IceInternal.LocalExceptionWrapper.throwWrapper(ex__);
                }
            }
        }

        public virtual string ice_id(Dictionary<string, string> context__, InvocationObserver obsv__)
        {
            Current current__ = new Current();
            initCurrent__(ref current__, ObjectPrxHelperBase.__ice_id_name, OperationMode.Nonmutating, context__);

            string result__ = null;
            IceInternal.Direct.RunDelegate run__ = delegate(Object servant__)
            {
                result__ = servant__.ice_id(current__);
                return Ice.DispatchStatus.DispatchOK;
            };

            IceInternal.Direct direct__ = null;
            try
            {
                direct__ = new IceInternal.Direct(current__, run__);
            }
            catch(System.Exception ex__)
            {
                IceInternal.LocalExceptionWrapper.throwWrapper(ex__);
            }

            try
            {
                DispatchStatus status__ = direct__.getServant().collocDispatch__(direct__);
                Debug.Assert(status__ == DispatchStatus.DispatchOK);
                return result__;
            }
            finally
            {
                try
                {
                    direct__.destroy();
                }
                catch(System.Exception ex__)
                {
                    IceInternal.LocalExceptionWrapper.throwWrapper(ex__);
                }
            }
        }

        public virtual bool ice_invoke(string operation, OperationMode mode, byte[] inEncaps,
                                       out byte[] outEncaps, Dictionary<string, string> context,
                                       InvocationObserver obsv)
        {
            throw new CollocationOptimizationException();
        }

        public virtual void ice_flushBatchRequests(InvocationObserver obsv)
        {
            throw new CollocationOptimizationException();
        }

        public virtual IceInternal.RequestHandler getRequestHandler__()
        {
            throw new CollocationOptimizationException();
        }

        public virtual void setRequestHandler__(IceInternal.RequestHandler handler)
        {
            throw new CollocationOptimizationException();
        }

        //
        // Only for use by ObjectPrx.
        //
        internal void copyFrom__(ObjectDelD_ from)
        {
            //
            // No need to synchronize "from", as the delegate is immutable
            // after creation.
            //

            //
            // No need to synchronize, as this operation is only called
            // upon initialization.
            //

            Debug.Assert(reference__ == null);
            Debug.Assert(adapter__ == null);

            reference__ = from.reference__;
            adapter__ = from.adapter__;
        }

        protected internal IceInternal.Reference reference__;
        protected internal ObjectAdapter adapter__;

        protected internal void initCurrent__(ref Current current, string op, OperationMode mode,
                                              Dictionary<string, string> context)
        {
            current.adapter = adapter__;
            current.id = reference__.getIdentity();
            current.facet = reference__.getFacet();
            current.operation = op;
            current.mode = mode;

            if(context != null)
            {
                current.ctx = context;
            }
            else
            {
                //
                // Implicit context
                //
                ImplicitContextI implicitContext =
                    reference__.getInstance().getImplicitContext();

                Dictionary<string, string> prxContext = reference__.getContext();

                if(implicitContext == null)
                {
                    current.ctx = new Dictionary<string, string>(prxContext);
                }
                else
                {
                    current.ctx = implicitContext.combine(prxContext);
                }
            }

            current.requestId = -1;
        }

        public virtual void setup(IceInternal.Reference rf, ObjectAdapter adapter)
        {
            //
            // No need to synchronize, as this operation is only called
            // upon initialization.
            //

            Debug.Assert(reference__ == null);
            Debug.Assert(adapter__ == null);

            reference__ = rf;
            adapter__ = adapter;
        }
    }

    public class ObjectDelM_ : ObjectDel_
    {
        public virtual bool ice_isA(string id__, Dictionary<string, string> context__, InvocationObserver obsv__)
        {
            IceInternal.Outgoing og__ = handler__.getOutgoing(ObjectPrxHelperBase.__ice_isA_name, 
                                                              OperationMode.Nonmutating, context__, obsv__);
            try
            {
                try
                {
                    IceInternal.BasicStream os__ = og__.startWriteParams(FormatType.DefaultFormat);
                    os__.writeString(id__);
                    og__.endWriteParams();
                }
                catch(LocalException ex__)
                {
                    og__.abort(ex__);
                }
                bool ok__ = og__.invoke();
                try
                {
                    if(!ok__)
                    {
                        try
                        {
                            og__.throwUserException();
                        }
                        catch(UserException ex)
                        {
                            throw new UnknownUserException(ex.ice_name(), ex);
                        }
                    }
                    IceInternal.BasicStream is__ = og__.startReadParams();
                    bool ret__ = is__.readBool();
                    og__.endReadParams();
                    return ret__;
                }
                catch(LocalException ex__)
                {
                    throw new IceInternal.LocalExceptionWrapper(ex__, false);
                }
            }
            finally
            {
                handler__.reclaimOutgoing(og__);
            }
        }

        public virtual void ice_ping(Dictionary<string, string> context__, InvocationObserver obsv__)
        {
            IceInternal.Outgoing og__ = handler__.getOutgoing(ObjectPrxHelperBase.__ice_ping_name, 
                                                              OperationMode.Nonmutating, context__, obsv__);
            try
            {
                og__.writeEmptyParams();
                bool ok__ = og__.invoke();
                if(og__.hasResponse())
                {
                    try
                    {
                        if(!ok__)
                        {
                            try
                            {
                                og__.throwUserException();
                            }
                            catch(UserException ex)
                            {
                                throw new UnknownUserException(ex.ice_name(), ex);
                            }
                        }
                        og__.readEmptyParams();
                    }
                    catch(LocalException ex__)
                    {
                        throw new IceInternal.LocalExceptionWrapper(ex__, false);
                    }
                }
            }
            finally
            {
                handler__.reclaimOutgoing(og__);
            }
        }

        public virtual string[] ice_ids(Dictionary<string, string> context__, InvocationObserver obsv__)
        {
            IceInternal.Outgoing og__ = handler__.getOutgoing(ObjectPrxHelperBase.__ice_ids_name, 
                                                              OperationMode.Nonmutating, context__, obsv__);
            try
            {
                og__.writeEmptyParams();
                bool ok__ = og__.invoke();
                try
                {
                    if(!ok__)
                    {
                        try
                        {
                            og__.throwUserException();
                        }
                        catch(UserException ex)
                        {
                            throw new UnknownUserException(ex.ice_name(), ex);
                        }
                    }
                    IceInternal.BasicStream is__ = og__.startReadParams();
                    string[] ret__ = is__.readStringSeq();
                    og__.endReadParams();
                    return ret__;
                }
                catch(LocalException ex__)
                {
                    throw new IceInternal.LocalExceptionWrapper(ex__, false);
                }
            }
            finally
            {
                handler__.reclaimOutgoing(og__);
            }
        }

        public virtual string ice_id(Dictionary<string, string> context__, InvocationObserver obsv__)
        {
            IceInternal.Outgoing og__ = handler__.getOutgoing(ObjectPrxHelperBase.__ice_id_name,
                                                              OperationMode.Nonmutating, context__, obsv__);
            try
            {
                og__.writeEmptyParams();
                bool ok__ = og__.invoke();
                try
                {
                    if(!ok__)
                    {
                        try
                        {
                            og__.throwUserException();
                        }
                        catch(UserException ex)
                        {
                            throw new UnknownUserException(ex.ice_name(), ex);
                        }
                    }
                    IceInternal.BasicStream is__ = og__.startReadParams();
                    string ret__ = is__.readString();
                    og__.endReadParams();
                    return ret__;
                }
                catch(LocalException ex__)
                {
                    throw new IceInternal.LocalExceptionWrapper(ex__, false);
                }
            }
            finally
            {
                handler__.reclaimOutgoing(og__);
            }
        }

        public virtual bool ice_invoke(string operation, OperationMode mode, byte[] inEncaps, out byte[] outEncaps,
                                       Dictionary<string, string> context__, InvocationObserver obsv__)
        {
            IceInternal.Outgoing og__ = handler__.getOutgoing(operation, mode, context__, obsv__);
            try
            {
                try
                {
                    og__.writeParamEncaps(inEncaps);
                }
                catch(LocalException ex__)
                {
                    og__.abort(ex__);
                }
                bool ok = og__.invoke();
                outEncaps = null;
                if(handler__.getReference().getMode() == IceInternal.Reference.Mode.ModeTwoway)
                {
                    try
                    {
                        outEncaps = og__.readParamEncaps();
                    }
                    catch(LocalException ex__)
                    {
                        throw new IceInternal.LocalExceptionWrapper(ex__, false);
                    }
                }
                return ok;
            }
            finally
            {
                handler__.reclaimOutgoing(og__);
            }
        }

        public virtual void ice_flushBatchRequests(InvocationObserver obsv)
        {
            IceInternal.BatchOutgoing @out = new IceInternal.BatchOutgoing(handler__, obsv);
            @out.invoke();
        }

        public virtual IceInternal.RequestHandler getRequestHandler__()
        {
            return handler__;
        }

        public virtual void setRequestHandler__(IceInternal.RequestHandler handler)
        {
            handler__ = handler;
        }

        //
        // Only for use by ObjectPrx
        //
        internal void copyFrom__(ObjectDelM_ from)
        {
            //
            // No need to synchronize "from", as the delegate is immutable
            // after creation.
            //

            //
            // No need to synchronize, as this operation is only called
            // upon initialization.
            //

            Debug.Assert(handler__ == null);

            handler__ = from.handler__;
        }

        protected IceInternal.RequestHandler handler__;

        public virtual void setup(IceInternal.Reference rf, ObjectPrx proxy, bool async)
        {
            //
            // No need to synchronize, as this operation is only called
            // upon initialization.
            //

            Debug.Assert(handler__ == null);

            if(async)
            {
                IceInternal.ConnectRequestHandler handler = new IceInternal.ConnectRequestHandler(rf, proxy, this);
                handler__ = handler.connect();
            }
            else
            {
                handler__ = new IceInternal.ConnectionRequestHandler(rf, proxy);
            }
        }
    }
}
