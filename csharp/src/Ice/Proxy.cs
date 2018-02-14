// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
    /// Delegate for a successful <code>ice_getConnection</code> invocation.
    /// <param name="ret__">The connection used by the proxy.</param>
    /// </summary>
    public delegate void Callback_Object_ice_getConnection(Connection ret__);

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
        /// Creates a new proxy that is identical to this proxy, except for the invocation timeout.
        /// </summary>
        /// <param name="timeout">The new invocation timeout (in seconds).</param>
        ObjectPrx ice_invocationTimeout(int timeout);

        /// <summary>
        /// Returns the invocation timeout of this proxy.
        /// </summary>
        /// <returns>The invocation timeout value (in seconds).</returns>
        int ice_getInvocationTimeout();

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
        /// Asynchronously gets the connection for this proxy.
        /// </summary>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult<Callback_Object_ice_getConnection> begin_ice_getConnection();

        /// <summary>
        /// Asynchronously gets the connection for this proxy.
        /// </summary>
        /// <param name="cb__">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie__">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_getConnection(AsyncCallback cb__, object cookie__);

        /// <summary>
        /// Asynchronously gets the connection for this proxy.
        /// </summary>
        /// <param name="r__">The asynchronous result object returned by <code>begin_ice_getConnection</code>.</param>
        /// <returns>The connection.</returns>
        Connection end_ice_getConnection(AsyncResult r__);

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

        private bool ice_isA(string id__, Dictionary<string, string> context__, bool explicitCtx__)
        {
            checkTwowayOnly__(__ice_isA_name);
            return end_ice_isA(begin_ice_isA(id__, context__, explicitCtx__, true, null, null));
        }

        public AsyncResult<Callback_Object_ice_isA> begin_ice_isA(string id)
        {
            return begin_ice_isA(id, null, false, false, null, null);
        }

        public AsyncResult<Callback_Object_ice_isA> begin_ice_isA(string id, Dictionary<string, string> context__)
        {
            return begin_ice_isA(id, context__, true, false, null, null);
        }

        public AsyncResult begin_ice_isA(string id, AsyncCallback cb__, object cookie__)
        {
            return begin_ice_isA(id, null, false, false, cb__, cookie__);
        }

        public AsyncResult begin_ice_isA(string id, Dictionary<string, string> context__, AsyncCallback cb__,
                                         object cookie__)
        {
            return begin_ice_isA(id, context__, true, false, cb__, cookie__);
        }

        internal const string __ice_isA_name = "ice_isA";

        public bool end_ice_isA(AsyncResult r__)
        {
            IceInternal.OutgoingAsync outAsync__ = IceInternal.OutgoingAsync.check(r__, this, __ice_isA_name);
            try
            {
                if(!outAsync__.wait())
                {
                    try
                    {
                        outAsync__.throwUserException();
                    }
                    catch(Ice.UserException ex__)
                    {
                        throw new Ice.UnknownUserException(ex__.ice_name(), ex__);
                    }
                }
                bool ret__;
                IceInternal.BasicStream is__ = outAsync__.startReadParams();
                ret__ = is__.readBool();
                outAsync__.endReadParams();
                return ret__;
            }
            finally
            {
                if(outAsync__ != null)
                {
                    outAsync__.cacheMessageBuffers();
                }
            }
        }

        private AsyncResult<Callback_Object_ice_isA> begin_ice_isA(string id,
                                                                   Dictionary<string, string> context__,
                                                                   bool explicitCtx__,
                                                                   bool synchronous__,
                                                                   Ice.AsyncCallback cb__,
                                                                   object cookie__)
        {
            checkAsyncTwowayOnly__(__ice_isA_name);

            IceInternal.TwowayOutgoingAsync<Callback_Object_ice_isA> result__ =
                getTwowayOutgoingAsync<Callback_Object_ice_isA>(__ice_isA_name, ice_isA_completed__, cookie__);
            if(cb__ != null)
            {
                result__.whenCompletedWithAsyncCallback(cb__);
            }

            try
            {
                result__.prepare(__ice_isA_name, OperationMode.Nonmutating, context__, explicitCtx__, synchronous__);
                IceInternal.BasicStream os__ = result__.startWriteParams(FormatType.DefaultFormat);
                os__.writeString(id);
                result__.endWriteParams();
                result__.invoke();
            }
            catch(Ice.Exception ex__)
            {
                result__.abort(ex__);
            }
            return result__;
        }

        protected IceInternal.TwowayOutgoingAsync<T>
        getTwowayOutgoingAsync<T>(string operation, IceInternal.ProxyTwowayCallback<T> cb,
            object cookie) {
            bool haveEntry = false;
            IceInternal.BasicStream iss = null;
            IceInternal.BasicStream os = null;

            if(_reference.getInstance().cacheMessageBuffers() > 0)
            {
                lock(this)
                {
                    if(_streamCache != null && _streamCache.Count > 0)
                    {
                        haveEntry = true;
                        iss = _streamCache.First.Value.iss;
                        os = _streamCache.First.Value.os;

                        _streamCache.RemoveFirst();
                    }
                }
            }
            if(!haveEntry)
            {
                return new IceInternal.TwowayOutgoingAsync<T>(this, operation, cb, cookie);
            }
            else
            {
                return new IceInternal.TwowayOutgoingAsync<T>(this, operation, cb, cookie, iss, os);
            }
        }

        protected IceInternal.OnewayOutgoingAsync<T>
        getOnewayOutgoingAsync<T>(string operation, IceInternal.ProxyOnewayCallback<T> cb,
            object cookie) {
            bool haveEntry = false;
            IceInternal.BasicStream iss = null;
            IceInternal.BasicStream os = null;

            if(_reference.getInstance().cacheMessageBuffers() > 0)
            {
                lock(this)
                {
                    if(_streamCache != null && _streamCache.Count > 0)
                    {
                        haveEntry = true;
                        iss = _streamCache.First.Value.iss;
                        os = _streamCache.First.Value.os;
                        _streamCache.RemoveFirst();
                    }
                }
            }
            if(!haveEntry)
            {
                return new IceInternal.OnewayOutgoingAsync<T>(this, operation, cb, cookie);
            }
            else
            {
                return new IceInternal.OnewayOutgoingAsync<T>(this, operation, cb, cookie, iss, os);
            }
        }

        public void
        cacheMessageBuffers(IceInternal.BasicStream iss, IceInternal.BasicStream os)
        {
            lock(this)
            {
                if(_streamCache == null)
                {
                    _streamCache = new LinkedList<StreamCacheEntry>();
                }
                StreamCacheEntry cacheEntry;
                cacheEntry.iss = iss;
                cacheEntry.os = os;
                _streamCache.AddLast(cacheEntry);
            }
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

        private void ice_ping(Dictionary<string, string> context__, bool explicitCtx__)
        {
            end_ice_ping(begin_ice_ping(context__, explicitCtx__, true, null, null));
        }

        public AsyncResult<Callback_Object_ice_ping> begin_ice_ping()
        {
            return begin_ice_ping(null, false, false, null, null);
        }

        public AsyncResult<Callback_Object_ice_ping> begin_ice_ping(Dictionary<string, string> context__)
        {
            return begin_ice_ping(context__, true, false, null, null);
        }

        public AsyncResult begin_ice_ping(AsyncCallback cb__, object cookie__)
        {
            return begin_ice_ping(null, false, false, cb__, cookie__);
        }

        public AsyncResult begin_ice_ping(Dictionary<string, string> context__, AsyncCallback cb__, object cookie__)
        {
            return begin_ice_ping(null, false, false, cb__, cookie__);
        }

        internal const string __ice_ping_name = "ice_ping";

        public void end_ice_ping(AsyncResult r__)
        {
            end__(r__, __ice_ping_name);
        }

        private AsyncResult<Callback_Object_ice_ping> begin_ice_ping(Dictionary<string, string> context__,
                                                                     bool explicitCtx__,
                                                                     bool synchronous__,
                                                                     Ice.AsyncCallback cb__,
                                                                     object cookie__)
        {
            IceInternal.OnewayOutgoingAsync<Callback_Object_ice_ping> result__ =
                getOnewayOutgoingAsync<Callback_Object_ice_ping>(__ice_ping_name, ice_ping_completed__, cookie__);
            if(cb__ != null)
            {
                result__.whenCompletedWithAsyncCallback(cb__);
            }

            try
            {
                result__.prepare(__ice_ping_name, OperationMode.Nonmutating, context__, explicitCtx__, synchronous__);
                result__.writeEmptyParams();
                result__.invoke();
            }
            catch(Ice.Exception ex__)
            {
                result__.abort(ex__);
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

        private string[] ice_ids(Dictionary<string, string> context__, bool explicitCtx__)
        {

            checkTwowayOnly__(__ice_ids_name);
            return end_ice_ids(begin_ice_ids(context__, explicitCtx__, true, null, null));
        }

        public AsyncResult<Callback_Object_ice_ids> begin_ice_ids()
        {
            return begin_ice_ids(null, false, false, null, null);
        }

        public AsyncResult<Callback_Object_ice_ids> begin_ice_ids(Dictionary<string, string> context__)
        {
            return begin_ice_ids(context__, true, false, null, null);
        }

        public AsyncResult begin_ice_ids(AsyncCallback cb__, object cookie__)
        {
            return begin_ice_ids(null, false, false, cb__, cookie__);
        }

        public AsyncResult begin_ice_ids(Dictionary<string, string> context__, AsyncCallback cb__, object cookie__)
        {
            return begin_ice_ids(null, false, false, cb__, cookie__);
        }

        internal const string __ice_ids_name = "ice_ids";

        public string[] end_ice_ids(AsyncResult r__)
        {
            IceInternal.OutgoingAsync outAsync__ = IceInternal.OutgoingAsync.check(r__, this, __ice_ids_name);
            try
            {
                if(!outAsync__.wait())
                {
                    try
                    {
                        outAsync__.throwUserException();
                    }
                    catch(Ice.UserException ex__)
                    {
                        throw new Ice.UnknownUserException(ex__.ice_name(), ex__);
                    }
                }
                string[] ret__;
                IceInternal.BasicStream is__ = outAsync__.startReadParams();
                ret__ = is__.readStringSeq();
                outAsync__.endReadParams();
                return ret__;
            }
            finally
            {
                if(outAsync__ != null)
                {
                    outAsync__.cacheMessageBuffers();
                }
            }
        }

        private AsyncResult<Callback_Object_ice_ids> begin_ice_ids(Dictionary<string, string> context__,
                                                                   bool explicitCtx__,
                                                                   bool synchronous__,
                                                                   Ice.AsyncCallback cb__,
                                                                   object cookie__)
        {
            checkAsyncTwowayOnly__(__ice_ids_name);

            IceInternal.TwowayOutgoingAsync<Callback_Object_ice_ids> result__ =
                getTwowayOutgoingAsync<Callback_Object_ice_ids>(__ice_ids_name, ice_ids_completed__, cookie__);
            if(cb__ != null)
            {
                result__.whenCompletedWithAsyncCallback(cb__);
            }

            try
            {
                result__.prepare(__ice_ids_name, OperationMode.Nonmutating, context__, explicitCtx__, synchronous__);
                result__.writeEmptyParams();
                result__.invoke();
            }
            catch(Ice.Exception ex__)
            {
                result__.abort(ex__);
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

        private string ice_id(Dictionary<string, string> context__, bool explicitCtx__)
        {
            checkTwowayOnly__(__ice_id_name);
            return end_ice_id(begin_ice_id(context__, explicitCtx__, true, null, null));
        }

        public AsyncResult<Callback_Object_ice_id> begin_ice_id()
        {
            return begin_ice_id(null, false, false, null, null);
        }

        public AsyncResult<Callback_Object_ice_id> begin_ice_id(Dictionary<string, string> context__)
        {
            return begin_ice_id(context__, true, false, null, null);
        }

        public AsyncResult begin_ice_id(AsyncCallback cb__, object cookie__)
        {
            return begin_ice_id(null, false, false, cb__, cookie__);
        }

        public AsyncResult begin_ice_id(Dictionary<string, string> context__, AsyncCallback cb__, object cookie__)
        {
            return begin_ice_id(null, false, false, cb__, cookie__);
        }

        internal const string __ice_id_name = "ice_id";

        public string end_ice_id(AsyncResult r__)
        {
            IceInternal.OutgoingAsync outAsync__ = IceInternal.OutgoingAsync.check(r__, this, __ice_id_name);
            try
            {
                if(!outAsync__.wait())
                {
                    try
                    {
                        outAsync__.throwUserException();
                    }
                    catch(Ice.UserException ex__)
                    {
                        throw new Ice.UnknownUserException(ex__.ice_name(), ex__);
                    }
                }
                string ret__;
                IceInternal.BasicStream is__ = outAsync__.startReadParams();
                ret__ = is__.readString();
                outAsync__.endReadParams();
                return ret__;
            }
            finally
            {
                if(outAsync__ != null)
                {
                    outAsync__.cacheMessageBuffers();
                }
            }
        }

        private AsyncResult<Callback_Object_ice_id> begin_ice_id(Dictionary<string, string> context__,
                                                                 bool explicitCtx__,
                                                                 bool synchronous__,
                                                                 Ice.AsyncCallback cb__,
                                                                 object cookie__)
        {
            checkAsyncTwowayOnly__(__ice_id_name);

            IceInternal.TwowayOutgoingAsync<Callback_Object_ice_id> result__ =
                getTwowayOutgoingAsync<Callback_Object_ice_id>(__ice_id_name, ice_id_completed__, cookie__);
            if(cb__ != null)
            {
                result__.whenCompletedWithAsyncCallback(cb__);
            }

            try
            {
                result__.prepare(__ice_id_name, OperationMode.Nonmutating, context__, explicitCtx__, synchronous__);
                result__.writeEmptyParams();
                result__.invoke();
            }
            catch(Ice.Exception ex__)
            {
                result__.abort(ex__);
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
                                Dictionary<string, string> context,  bool explicitCtx)
        {
            return end_ice_invoke(out outEncaps, begin_ice_invoke(operation, mode, inEncaps, context, explicitCtx, true, null, null));
        }

        public AsyncResult<Callback_Object_ice_invoke> begin_ice_invoke(string operation,
                                                                        OperationMode mode,
                                                                        byte[] inEncaps)
        {
            return begin_ice_invoke(operation, mode, inEncaps, null, false, false, null, null);
        }

        public AsyncResult<Callback_Object_ice_invoke> begin_ice_invoke(string operation,
                                                                        OperationMode mode,
                                                                        byte[] inEncaps,
                                                                        Dictionary<string, string> context__)
        {
            return begin_ice_invoke(operation, mode, inEncaps, context__, true, false, null, null);
        }

        public AsyncResult begin_ice_invoke(string operation, OperationMode mode, byte[] inEncaps, AsyncCallback cb__,
                                            object cookie__)
        {
            return begin_ice_invoke(operation, mode, inEncaps, null, false, false, cb__, cookie__);
        }

        public AsyncResult begin_ice_invoke(string operation, OperationMode mode, byte[] inEncaps,
                                            Dictionary<string, string> context__, AsyncCallback cb__, object cookie__)
        {
            return begin_ice_invoke(operation, mode, inEncaps, null, false, false, cb__, cookie__);
        }

        internal const string __ice_invoke_name = "ice_invoke";

        public bool end_ice_invoke(out byte[] outEncaps, AsyncResult r__)
        {
            IceInternal.OutgoingAsync outAsync__ = IceInternal.OutgoingAsync.check(r__, this, __ice_invoke_name);
            try
            {
                bool ok = outAsync__.wait();
                if(_reference.getMode() == IceInternal.Reference.Mode.ModeTwoway)
                {
                    outEncaps = outAsync__.readParamEncaps();
                }
                else
                {
                    outEncaps = null; // Satisfy compiler
                }
                return ok;
            }
            finally
            {
                if(outAsync__ != null)
                {
                    outAsync__.cacheMessageBuffers();
                }
            }

        }

        private AsyncResult<Callback_Object_ice_invoke> begin_ice_invoke(string operation,
                                                                         OperationMode mode,
                                                                         byte[] inEncaps,
                                                                         Dictionary<string, string> context__,
                                                                         bool explicitCtx__,
                                                                         bool synchronous__,
                                                                         Ice.AsyncCallback cb__,
                                                                         object cookie__)
        {
            IceInternal.TwowayOutgoingAsync<Callback_Object_ice_invoke> result__ =
                getTwowayOutgoingAsync<Callback_Object_ice_invoke>(__ice_invoke_name, ice_invoke_completed__, cookie__);
            if(cb__ != null)
            {
                result__.whenCompletedWithAsyncCallback(cb__);
            }

            try
            {
                result__.prepare(operation, mode, context__, explicitCtx__, synchronous__);
                result__.writeParamEncaps(inEncaps);
                result__.invoke();
            }
            catch(Ice.Exception ex__)
            {
                result__.abort(ex__);
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
            if(newTimeout < -1)
            {
                throw new System.ArgumentException("invalid value passed to ice_locatorCacheTimeout: " + newTimeout);
            }
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
        /// Returns the invocation timeout of this proxy.
        /// </summary>
        /// <returns>The invocation timeout value (in seconds).</returns>
        public int ice_getInvocationTimeout()
        {
            return _reference.getInvocationTimeout();
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the invocation timeout.
        /// </summary>
        /// <param name="newTimeout">The new invocation timeout (in seconds).</param>
        public ObjectPrx ice_invocationTimeout(int newTimeout)
        {
            if(newTimeout < 1 && newTimeout != -1 && newTimeout != -2)
            {
                throw new System.ArgumentException("invalid value passed to ice_invocationTimeout: " + newTimeout);
            }
            if(newTimeout == _reference.getInvocationTimeout())
            {
                return this;
            }
            else
            {
                return newInstance(_reference.changeInvocationTimeout(newTimeout));
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
            if(t < 1 && t != -1)
            {
                throw new System.ArgumentException("invalid value passed to ice_timeout: " + t);
            }
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
            return end_ice_getConnection(begin_ice_getConnection());
        }

        public AsyncResult<Callback_Object_ice_getConnection> begin_ice_getConnection()
        {
            return begin_ice_getConnectionInternal(null, null);
        }

        internal const string __ice_getConnection_name = "ice_getConnection";

        public AsyncResult begin_ice_getConnection(Ice.AsyncCallback cb, object cookie)
        {
            return begin_ice_getConnectionInternal(cb, cookie);
        }

        public Connection end_ice_getConnection(Ice.AsyncResult r)
        {
            IceInternal.ProxyGetConnection outAsync =
                IceInternal.ProxyGetConnection.check(r, this, __ice_getConnection_name);
            outAsync.wait();
            return ice_getCachedConnection();
        }

        private AsyncResult<Callback_Object_ice_getConnection> begin_ice_getConnectionInternal(Ice.AsyncCallback cb,
                                                                                               object cookie)
        {
            IceInternal.ProxyGetConnection result = new IceInternal.ProxyGetConnection(this,
                                                                                       __ice_getConnection_name,
                                                                                       ice_getConnection_completed__,
                                                                                       cookie);
            if(cb != null)
            {
                result.whenCompletedWithAsyncCallback(cb);
            }
            try
            {
                result.invoke();
            }
            catch(Ice.Exception ex)
            {
                result.abort(ex);
            }
            return result;
        }



        private void ice_getConnection_completed__(AsyncResult r,
                                                   Callback_Object_ice_getConnection cb,
                                                   Ice.ExceptionCallback excb)
        {
            Connection ret;
            try
            {
                ret = end_ice_getConnection(r);
            }
            catch(Ice.Exception ex)
            {
                if(excb != null)
                {
                    excb(ex);
                }
                return;
            }
            if(cb != null)
            {
                cb(ret);
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
            IceInternal.RequestHandler handler;
            lock(this)
            {
                handler = _requestHandler;
            }

            if(handler != null)
            {
                try
                {
                    return handler.getConnection();
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
            end_ice_flushBatchRequests(begin_ice_flushBatchRequests());
        }

        internal const string __ice_flushBatchRequests_name = "ice_flushBatchRequests";

        public AsyncResult begin_ice_flushBatchRequests()
        {
            return begin_ice_flushBatchRequests(null, null);
        }

        public AsyncResult begin_ice_flushBatchRequests(Ice.AsyncCallback cb, object cookie)
        {
            IceInternal.ProxyFlushBatch result = new IceInternal.ProxyFlushBatch(this,
                                                                                 __ice_flushBatchRequests_name,
                                                                                 cookie);
            if(cb != null)
            {
                result.whenCompletedWithAsyncCallback(cb);
            }
            try
            {
                result.invoke();
            }
            catch(Ice.Exception ex)
            {
                result.abort(ex);
            }
            return result;
        }

        public void end_ice_flushBatchRequests(Ice.AsyncResult r)
        {
            IceInternal.ProxyFlushBatch outAsync =
                IceInternal.ProxyFlushBatch.check(r, this, __ice_flushBatchRequests_name);
            outAsync.wait();
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
            lock(from)
            {
                ObjectPrxHelperBase h = (ObjectPrxHelperBase)from;
                _reference = h._reference;
                _requestHandler = h._requestHandler;
            }
        }

        public int handleException__(Exception ex, IceInternal.RequestHandler handler, OperationMode mode, bool sent,
                                     ref int cnt)
        {
            updateRequestHandler__(handler, null); // Clear the request handler

            //
            // We only retry local exception, system exceptions aren't retried.
            //
            // A CloseConnectionException indicates graceful server shutdown, and is therefore
            // always repeatable without violating "at-most-once". That's because by sending a
            // close connection message, the server guarantees that all outstanding requests
            // can safely be repeated.
            //
            // An ObjectNotExistException can always be retried as well without violating
            // "at-most-once" (see the implementation of the checkRetryAfterException method
            //  of the ProxyFactory class for the reasons why it can be useful).
            //
            // If the request didn't get sent or if it's non-mutating or idempotent it can
            // also always be retried if the retry count isn't reached.
            //
            if(ex is LocalException && (!sent ||
                                        mode == OperationMode.Nonmutating || mode == OperationMode.Idempotent ||
                                        ex is CloseConnectionException ||
                                        ex is ObjectNotExistException))
            {
                try
                {
                    return _reference.getInstance().proxyFactory().checkRetryAfterException((LocalException)ex,
                                                                                            _reference,
                                                                                            ref cnt);
                }
                catch(CommunicatorDestroyedException)
                {
                    //
                    // The communicator is already destroyed, so we cannot retry.
                    //
                    throw ex;
                }
            }
            else
            {
                throw ex; // Retry could break at-most-once semantics, don't retry.
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

        public void end__(AsyncResult r, string operation)
        {
            IceInternal.ProxyOutgoingAsyncBase result = IceInternal.ProxyOutgoingAsyncBase.check(r, this, operation);
            try
            {
                bool ok = result.wait();
                if(_reference.getMode() == IceInternal.Reference.Mode.ModeTwoway)
                {
                    IceInternal.OutgoingAsync outAsync = (IceInternal.OutgoingAsync)result;
                    if(!ok)
                    {
                        try
                        {
                            outAsync.throwUserException();
                        }
                        catch(Ice.UserException ex)
                        {
                            throw new Ice.UnknownUserException(ex.ice_name(), ex);
                        }
                    }
                    outAsync.readEmptyParams();
                }
            }
            finally
            {
                if(result != null)
                {
                    result.cacheMessageBuffers();
                }
            }
        }

        public IceInternal.RequestHandler getRequestHandler__()
        {
            if(_reference.getCacheConnection())
            {
                lock(this)
                {
                    if(_requestHandler != null)
                    {
                        return _requestHandler;
                    }
                }
            }
            return _reference.getRequestHandler(this);
        }

        public IceInternal.BatchRequestQueue
        getBatchRequestQueue__()
        {
            lock(this)
            {
                if(_batchRequestQueue == null)
                {
                    _batchRequestQueue = _reference.getBatchRequestQueue();
                }
                return _batchRequestQueue;
            }
        }

        public IceInternal.RequestHandler
        setRequestHandler__(IceInternal.RequestHandler handler)
        {
            if(_reference.getCacheConnection())
            {
                lock(this)
                {
                    if(_requestHandler == null)
                    {
                        _requestHandler = handler;
                    }
                    return _requestHandler;
                }
            }
            return handler;
        }

        public void updateRequestHandler__(IceInternal.RequestHandler previous, IceInternal.RequestHandler handler)
        {
            if(_reference.getCacheConnection() && previous != null)
            {
                lock(this)
                {
                    if(_requestHandler != null && _requestHandler != handler)
                    {
                        //
                        // Update the request handler only if "previous" is the same
                        // as the current request handler. This is called after
                        // connection binding by the connect request handler. We only
                        // replace the request handler if the current handler is the
                        // connect request handler.
                        //
                        _requestHandler = _requestHandler.update(previous, handler);
                    }
                }
            }
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
            Debug.Assert(_requestHandler == null);

            _reference = @ref;
        }

        private ObjectPrxHelperBase newInstance(IceInternal.Reference @ref)
        {
            ObjectPrxHelperBase proxy = (ObjectPrxHelperBase)Activator.CreateInstance(GetType());
            proxy.setup(@ref);
            return proxy;
        }

        private IceInternal.Reference _reference;
        private IceInternal.RequestHandler _requestHandler;
        private IceInternal.BatchRequestQueue _batchRequestQueue;
        private struct StreamCacheEntry
        {
            public IceInternal.BasicStream iss;
            public IceInternal.BasicStream os;
        }

        private LinkedList<StreamCacheEntry> _streamCache;
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


        /// <summary>
        /// Returns the Slice type id of the interface or class associated
        /// with this proxy class.
        /// </summary>
        /// <returns>The type id, "::Ice::Object".</returns>
        public static string ice_staticId()
        {
            return Ice.ObjectImpl.ice_staticId();
        }
    }
}
