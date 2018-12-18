// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.Serialization;
using System.Threading;
using System.Threading.Tasks;
using System.ComponentModel;

using IceUtilInternal;
using IceInternal;

namespace Ice
{
    /// <summary>
    /// Delegate for a successful <code>ice_isA</code> invocation.
    /// <param name="ret">True if the remote object supports the type, false otherwise.</param>
    /// </summary>
    public delegate void Callback_Object_ice_isA(bool ret);

    /// <summary>
    /// Delegate for a successful <code>ice_ids</code> invocation.
    /// <param name="ret">The array of Slice type ids supported by the remote object.</param>
    /// </summary>
    public delegate void Callback_Object_ice_ids(string[] ret);

    /// <summary>
    /// Delegate for a successful <code>ice_id</code> invocation.
    /// <param name="ret">The Slice type id of the most-derived interface supported by the remote object.</param>
    /// </summary>
    public delegate void Callback_Object_ice_id(string ret);

    /// <summary>
    /// Delegate for a successful <code>ice_ping</code> invocation.
    /// </summary>
    public delegate void Callback_Object_ice_ping();

    /// <summary>
    /// Delegate for a successful <code>ice_invoke</code> invocation.
    /// <param name="ret">True if the invocation succeeded, or false if the invocation
    /// raised a user exception.</param>
    /// <param name="outEncaps">The encoded out-parameters or user exception.</param>
    /// </summary>
    public delegate void Callback_Object_ice_invoke(bool ret, byte[] outEncaps);

    /// <summary>
    /// Delegate for a successful <code>ice_getConnection</code> invocation.
    /// <param name="ret">The connection used by the proxy.</param>
    /// </summary>
    public delegate void Callback_Object_ice_getConnection(Connection ret);

    /// <summary>
    /// Value type to allow differenciate between a context that is explicity set to
    /// empty (empty or null dictionary) and a context that has non been set.
    /// </summary>
    public struct OptionalContext
    {
        private OptionalContext(Dictionary<string, string> ctx)
        {
            _ctx = ctx == null ? _emptyContext : ctx;
        }

        /// <summary>
        /// Implicit conversion between Dictionary&lt;string, string&gt; and
        /// OptionalContext.
        /// </summary>
        /// <param name="ctx">Dictionary to convert.</param>
        /// <returns>OptionalContext value representing the dictionary</returns>
        public static implicit operator OptionalContext(Dictionary<string, string> ctx)
        {
            return new OptionalContext(ctx);
        }

        /// <summary>
        /// Implicit conversion between OptionalContext and
        /// Dictionary&lt;string, string&gt;
        /// </summary>
        /// <param name="value">OptionalContext value to convert</param>
        /// <returns>The Dictionary object.</returns>
        public static implicit operator Dictionary<string, string>(OptionalContext value)
        {
            return value._ctx;
        }

        private Dictionary<string, string> _ctx;
        static private Dictionary<string, string> _emptyContext = new Dictionary<string, string>();
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
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>True if the target object has the interface specified by id or derives
        /// from the interface specified by id.</returns>
        bool ice_isA(string id, OptionalContext context = new OptionalContext());

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        Task<bool> ice_isAAsync(string id,
                                OptionalContext context = new OptionalContext(),
                                IProgress<bool> progress = null,
                                CancellationToken cancel = new CancellationToken());
        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_isA(string id, AsyncCallback callback, object cookie);

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_isA(string id, OptionalContext context, AsyncCallback callback, object cookie);

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult<Callback_Object_ice_isA> begin_ice_isA(string id, OptionalContext context = new OptionalContext());

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="result">The asynchronous result object returned by <code>begin_ice_isA</code>.</param>
        /// <returns>True if the object supports the Slice interface, false otherwise.</returns>
        bool end_ice_isA(AsyncResult result);

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        void ice_ping(OptionalContext context = new OptionalContext());

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        Task ice_pingAsync(OptionalContext context = new OptionalContext(),
                           IProgress<bool> progress = null,
                           CancellationToken cancel = new CancellationToken());

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_ping(AsyncCallback callback, object cookie);

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_ping(OptionalContext context, AsyncCallback callback, object cookie);

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult<Callback_Object_ice_ping> begin_ice_ping(OptionalContext context = new OptionalContext());

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="result">The asynchronous result object returned by <code>begin_ice_ping</code>.</param>
        void end_ice_ping(AsyncResult result);

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>The Slice type IDs of the interfaces supported by the target object, in base-to-derived
        /// order. The first element of the returned array is always ::Ice::Object.</returns>
        string[] ice_ids(OptionalContext context = new OptionalContext());

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        Task<string[]> ice_idsAsync(OptionalContext context = new OptionalContext(),
                                    IProgress<bool> progress = null,
                                    CancellationToken cancel = new CancellationToken());

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_ids(AsyncCallback callback, object cookie);

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_ids(OptionalContext context, AsyncCallback callback, object cookie);

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult<Callback_Object_ice_ids> begin_ice_ids(OptionalContext context = new OptionalContext());

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="result">The asynchronous result object returned by <code>begin_ice_ids</code>.</param>
        /// <returns>The Slice type IDs of the interfaces supported by the target object, in base-to-derived
        /// order. The first element of the returned array is always ::Ice::Object.</returns>
        string[] end_ice_ids(AsyncResult result);

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        string ice_id(OptionalContext context = new OptionalContext());

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        Task<string> ice_idAsync(OptionalContext context = new OptionalContext(),
                                 IProgress<bool> progress = null,
                                 CancellationToken cancel = new CancellationToken());

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_id(AsyncCallback callback, object cookie);

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_id(OptionalContext context, AsyncCallback callback, object cookie);

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult<Callback_Object_ice_id> begin_ice_id(OptionalContext context = new OptionalContext());

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="result">The asynchronous result object returned by <code>begin_ice_id</code>.</param>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        string end_ice_id(AsyncResult result);

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
        bool ice_invoke(string operation, OperationMode mode, byte[] inEncaps, out byte[] outEncaps,
                        OptionalContext context = new OptionalContext());

        /// <summary>
        /// Invokes an operation dynamically.
        /// </summary>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        Task<Object_Ice_invokeResult>
        ice_invokeAsync(string operation,
                        OperationMode mode,
                        byte[] inEncaps,
                        OptionalContext context = new OptionalContext(),
                        IProgress<bool> progress = null,
                        CancellationToken cancel = new CancellationToken());

        /// <summary>
        /// Invokes an operation dynamically.
        /// </summary>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_invoke(string operation,
                                     OperationMode mode,
                                     byte[] inEncaps,
                                     AsyncCallback callback,
                                     object cookie);

        /// <summary>
        /// Invokes an operation dynamically.
        /// </summary>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_invoke(string operation,
                                     OperationMode mode,
                                     byte[] inEncaps,
                                     OptionalContext context,
                                     AsyncCallback callback,
                                     object cookie);

        /// <summary>
        /// Invokes an operation dynamically.
        /// </summary>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult<Callback_Object_ice_invoke> begin_ice_invoke(string operation,
                                                                 OperationMode mode,
                                                                 byte[] inEncaps,
                                                                 OptionalContext context = new OptionalContext());

        /// <summary>
        /// Completes a dynamic invocation.
        /// </summary>
        /// <param name="outEncaps">The encoded out parameters or user exception.</param>
        /// <param name="result">The asynchronous result object returned by <code>begin_ice_invoke</code>.</param>
        /// <returns>If the operation completed successfully, the return value
        /// is true. If the operation raises a user exception,
        /// the return value is false; in this case, outEncaps
        /// contains the encoded user exception. If the operation raises a run-time exception,
        /// it throws it directly.</returns>
        bool end_ice_invoke(out byte[] outEncaps, AsyncResult result);

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
        ObjectPrx ice_encodingVersion(EncodingVersion e);

        /// <summary>Returns the encoding version used to marshal requests parameters.</summary>
        /// <returns>The encoding version.</returns>
        EncodingVersion ice_getEncodingVersion();

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
        RouterPrx ice_getRouter();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the router.
        /// </summary>
        /// <param name="router">The router for the new proxy.</param>
        /// <returns>The new proxy with the specified router.</returns>
        ObjectPrx ice_router(RouterPrx router);

        /// <summary>
        /// Returns the locator for this proxy.
        /// </summary>
        /// <returns>The locator for this proxy. If no locator is configured, the return value is null.</returns>
        LocatorPrx ice_getLocator();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the locator.
        /// </summary>
        /// <param name="locator">The locator for the new proxy.</param>
        /// <returns>The new proxy with the specified locator.</returns>
        ObjectPrx ice_locator(LocatorPrx locator);

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
        /// Obtains the compression override setting of this proxy.
        /// </summary>
        /// <returns>The compression override setting. If no optional value is present, no override is
        /// set. Otherwise, true if compression is enabled, false otherwise.</returns>
        Ice.Optional<bool> ice_getCompress();

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for its timeout setting.
        /// </summary>
        /// <param name="t">The timeout for the new proxy in milliseconds.</param>
        /// <returns>A new proxy with the specified timeout.</returns>
        ObjectPrx ice_timeout(int t);

        /// <summary>
        /// Obtains the timeout override of this proxy.
        /// </summary>
        /// <returns>The timeout override. If no optional value is present, no override is set. Otherwise,
        /// returns the timeout override value.</returns>
        Ice.Optional<int> ice_getTimeout();

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
        /// Returns a proxy that is identical to this proxy, except it's a fixed proxy bound
        /// the given connection.
        /// </summary>
        /// <param name="connection">The fixed proxy connection.</param>
        /// <returns>A fixed proxy bound to the given connection.</returns>
        ObjectPrx ice_fixed(Ice.Connection connection);

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
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        Task<Connection> ice_getConnectionAsync(IProgress<bool> progress = null,
                                                CancellationToken cancel = new CancellationToken());

        /// <summary>
        /// Asynchronously gets the connection for this proxy.
        /// </summary>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult begin_ice_getConnection(AsyncCallback callback, object cookie);

        /// <summary>
        /// Asynchronously gets the connection for this proxy.
        /// </summary>
        /// <returns>An asynchronous result object.</returns>
        AsyncResult<Callback_Object_ice_getConnection> begin_ice_getConnection();

        /// <summary>
        /// Asynchronously gets the connection for this proxy.
        /// </summary>
        /// <param name="result">The asynchronous result object returned by <code>begin_ice_getConnection</code>.</param>
        /// <returns>The connection.</returns>
        Connection end_ice_getConnection(AsyncResult result);

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
        /// Flushes any pending batched requests for this proxy. The call blocks until the flush is complete.
        /// </summary>
        void ice_flushBatchRequests();

        /// <summary>
        /// Asynchronously flushes any pending batched requests for this proxy.
        /// </summary>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        Task ice_flushBatchRequestsAsync(IProgress<bool> progress = null,
                                         CancellationToken cancel = new CancellationToken());

        /// <summary>
        /// Asynchronously flushes any pending batched requests for this proxy.
        /// </summary>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        AsyncResult begin_ice_flushBatchRequests(AsyncCallback callback = null, object cookie = null);

        /// <summary>
        /// Flushes any pending batched requests for this proxy. The call blocks until the flush is complete.
        /// </summary>
        /// <param name="result">The asynchronous result object returned by <code>begin_ice_getConnection</code>.</param>
        void end_ice_flushBatchRequests(AsyncResult result);

        /// <summary>
        /// Write a proxy to the output stream.
        /// </summary>
        /// <param name="os">Output stream object to write the proxy.</param>
        [EditorBrowsable(EditorBrowsableState.Never)]
        void iceWrite(OutputStream os);

        /// <summary>
        /// Returns a scheduler object that uses the Ice thread pool.
        /// </summary>
        /// <returns>The task scheduler object.</returns>
        System.Threading.Tasks.TaskScheduler ice_scheduler();
    }

    /// <summary>
    /// Represent the result of the ice_invokeAsync operation
    /// </summary>
    public struct Object_Ice_invokeResult
    {
        public Object_Ice_invokeResult(bool returnValue, byte[] outEncaps)
        {
            this.returnValue = returnValue;
            this.outEncaps = outEncaps;
        }

        /// <summary>
        /// If the operation completed successfully, the return value
        /// is true. If the operation raises a user exception,
        /// the return value is false; in this case, outEncaps
        /// contains the encoded user exception.
        /// </summary>
        public bool returnValue;

        /// <summary>
        /// The encoded out-paramaters and return value for the operation.
        /// The return value follows any out-parameters. If returnValue is
        /// false it contains the encoded user exception.
        /// </summary>
        public byte[] outEncaps;
    };

    /// <summary>
    /// Base class of all object proxies.
    /// </summary>
    [Serializable]
    public class ObjectPrxHelperBase : ObjectPrx, ISerializable
    {
        public ObjectPrxHelperBase()
        {
        }

        public ObjectPrxHelperBase(SerializationInfo info, StreamingContext context)
        {
            Instance instance = null;

            if(context.Context is Communicator)
            {
                instance = IceInternal.Util.getInstance(context.Context as Communicator);
            }
            else if(context.Context is Instance)
            {
                instance = context.Context as Instance;
            }
            else
            {
                throw new ArgumentException("Cannot deserialize proxy: Ice.Communicator not found in StreamingContext");
            }

            var proxy = (ObjectPrxHelperBase)instance.proxyFactory().stringToProxy(info.GetString("proxy"));
            _reference = proxy._reference;
            Debug.Assert(proxy._requestHandler == null);
        }

        public virtual void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            info.AddValue("proxy", ToString());
        }

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
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>True if the target object has the interface specified by id or derives
        /// from the interface specified by id.</returns>
        public bool ice_isA(string id, OptionalContext context = new OptionalContext())
        {
            try
            {
                return iceI_ice_isAAsync(id, context, null, CancellationToken.None, true).Result;
            }
            catch(AggregateException ex)
            {
                throw ex.InnerException;
            }
        }

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task<bool> ice_isAAsync(string id,
                                       OptionalContext context = new OptionalContext(),
                                       IProgress<bool> progress = null,
                                       CancellationToken cancel = new CancellationToken())
        {
            return iceI_ice_isAAsync(id, context, progress, cancel, false);
        }

        private Task<bool>
        iceI_ice_isAAsync(string id, OptionalContext context, IProgress<bool> progress, CancellationToken cancel,
                          bool synchronous)
        {
            iceCheckTwowayOnly(_ice_isA_name);
            var completed = new OperationTaskCompletionCallback<bool>(progress, cancel);
            iceI_ice_isA(id, context, completed, synchronous);
            return completed.Task;
        }

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        public AsyncResult begin_ice_isA(string id, AsyncCallback callback, object cookie)
        {
            return iceI_begin_ice_isA(id, new OptionalContext(), callback, cookie, false);
        }

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        public AsyncResult begin_ice_isA(string id, OptionalContext context, AsyncCallback callback, object cookie)
        {
            return iceI_begin_ice_isA(id, context, callback, cookie, false);
        }

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>An asynchronous result object.</returns>
        public AsyncResult<Callback_Object_ice_isA>
        begin_ice_isA(string id, OptionalContext context = new OptionalContext())
        {
            return iceI_begin_ice_isA(id, context, null, null, false);
        }

        private const string _ice_isA_name = "ice_isA";

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="result">The asynchronous result object returned by <code>begin_ice_isA</code>.</param>
        /// <returns>True if the object supports the Slice interface, false otherwise.</returns>
        public bool end_ice_isA(AsyncResult result)
        {
            var resultI = AsyncResultI.check(result, this, _ice_isA_name);
            return ((OutgoingAsyncT<bool>)resultI.OutgoingAsync).getResult(resultI.wait());
        }

        private AsyncResult<Callback_Object_ice_isA>
        iceI_begin_ice_isA(string id, Dictionary<string, string> context, AsyncCallback callback, object cookie,
                           bool synchronous)
        {
            iceCheckAsyncTwowayOnly(_ice_isA_name);
            var completed = new OperationAsyncResultCompletionCallback<Callback_Object_ice_isA, bool>(
                (Callback_Object_ice_isA cb, bool result) =>
                {
                    if(cb != null)
                    {
                        cb.Invoke(result);
                    }
                }, this, _ice_isA_name, cookie, callback);
            iceI_ice_isA(id, context, completed, synchronous);
            return completed;
        }

        private void iceI_ice_isA(string id,
                                  Dictionary<string, string> context,
                                  OutgoingAsyncCompletionCallback completed,
                                  bool synchronous)
        {
            iceCheckAsyncTwowayOnly(_ice_isA_name);
            getOutgoingAsync<bool>(completed).invoke(_ice_isA_name,
                                                     OperationMode.Nonmutating,
                                                     FormatType.DefaultFormat,
                                                     context,
                                                     synchronous,
                                                     (OutputStream os) => { os.writeString(id); },
                                                     null,
                                                     (InputStream iss) => { return iss.readBool(); });
        }

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        public void ice_ping(OptionalContext context = new OptionalContext())
        {
            try
            {
                iceI_ice_pingAsync(context, null, CancellationToken.None, true).Wait();
            }
            catch(AggregateException ex)
            {
                throw ex.InnerException;
            }
        }

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task ice_pingAsync(OptionalContext context = new OptionalContext(),
                                  IProgress<bool> progress = null,
                                  CancellationToken cancel = new CancellationToken())
        {
            return iceI_ice_pingAsync(context, progress, cancel, false);
        }

        private Task
        iceI_ice_pingAsync(OptionalContext context, IProgress<bool> progress, CancellationToken cancel, bool synchronous)
        {
            var completed = new OperationTaskCompletionCallback<object>(progress, cancel);
            iceI_ice_ping(context, completed, synchronous);
            return completed.Task;
        }

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        public AsyncResult begin_ice_ping(AsyncCallback callback, object cookie)
        {
            return iceI_begin_ice_ping(new OptionalContext(), callback, cookie, false);
        }

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        public AsyncResult begin_ice_ping(OptionalContext context, AsyncCallback callback, object cookie)
        {
            return iceI_begin_ice_ping(context, callback, cookie, false);
        }

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>An asynchronous result object.</returns>
        public AsyncResult<Callback_Object_ice_ping>
        begin_ice_ping(OptionalContext context = new OptionalContext())
        {
            return iceI_begin_ice_ping(context, null, null, false);
        }

        private const string _ice_ping_name = "ice_ping";

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="result">The asynchronous result object returned by <code>begin_ice_ping</code>.</param>
        public void end_ice_ping(AsyncResult result)
        {
            var resultI = AsyncResultI.check(result, this, _ice_ping_name);
            ((OutgoingAsyncT<object>)resultI.OutgoingAsync).getResult(resultI.wait());
        }

        private AsyncResult<Callback_Object_ice_ping> iceI_begin_ice_ping(Dictionary<string, string> context,
                                                                          AsyncCallback callback,
                                                                          object cookie,
                                                                          bool synchronous)
        {
            var completed = new OperationAsyncResultCompletionCallback<Callback_Object_ice_ping, object>(
                (Callback_Object_ice_ping cb, object result) =>
                {
                    if(cb != null)
                    {
                        cb.Invoke();
                    }
                }, this, _ice_ping_name, cookie, callback);
            iceI_ice_ping(context, completed, synchronous);
            return completed;
        }

        private void iceI_ice_ping(Dictionary<string, string> context, OutgoingAsyncCompletionCallback completed,
                                       bool synchronous)
        {
            getOutgoingAsync<object>(completed).invoke(_ice_ping_name,
                                                       OperationMode.Nonmutating,
                                                       FormatType.DefaultFormat,
                                                       context,
                                                       synchronous);
        }

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>The Slice type IDs of the interfaces supported by the target object, in base-to-derived
        /// order. The first element of the returned array is always ::Ice::Object.</returns>
        public string[] ice_ids(OptionalContext context = new OptionalContext())
        {
            try
            {
                return iceI_ice_idsAsync(context, null, CancellationToken.None, true).Result;
            }
            catch(AggregateException ex)
            {
                throw ex.InnerException;
            }
        }

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task<string[]>
        ice_idsAsync(OptionalContext context = new OptionalContext(),
                     IProgress<bool> progress = null,
                     CancellationToken cancel = new CancellationToken())
        {
            return iceI_ice_idsAsync(context, progress, cancel, false);
        }

        private Task<string[]> iceI_ice_idsAsync(OptionalContext context, IProgress<bool> progress, CancellationToken cancel,
                                                 bool synchronous)
        {
            iceCheckTwowayOnly(_ice_ids_name);
            var completed = new OperationTaskCompletionCallback<string[]>(progress, cancel);
            iceI_ice_ids(context, completed, false);
            return completed.Task;
        }

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        public AsyncResult begin_ice_ids(AsyncCallback callback, object cookie)
        {
            return iceI_begin_ice_ids(new OptionalContext(), callback, cookie, false);
        }

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        public AsyncResult
        begin_ice_ids(OptionalContext context, AsyncCallback callback, object cookie)
        {
            return iceI_begin_ice_ids(context, callback, cookie, false);
        }

        public AsyncResult<Callback_Object_ice_ids>
        begin_ice_ids(OptionalContext context = new OptionalContext())
        {
            return iceI_begin_ice_ids(context, null, null, false);
        }

        private const string _ice_ids_name = "ice_ids";

        private AsyncResult<Callback_Object_ice_ids>
        iceI_begin_ice_ids(Dictionary<string, string> context, AsyncCallback callback, object cookie, bool synchronous)
        {
            iceCheckAsyncTwowayOnly(_ice_ids_name);
            var completed = new OperationAsyncResultCompletionCallback<Callback_Object_ice_ids, string[]>(
                (Callback_Object_ice_ids cb, string[] result) =>
                {
                    if(cb != null)
                    {
                        cb.Invoke(result);
                    }
                }, this, _ice_ids_name, cookie, callback);
            iceI_ice_ids(context, completed, synchronous);
            return completed;
        }

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="result">The asynchronous result object returned by <code>begin_ice_ids</code>.</param>
        /// <returns>The Slice type IDs of the interfaces supported by the target object, in base-to-derived
        /// order. The first element of the returned array is always ::Ice::Object.</returns>
        public string[] end_ice_ids(AsyncResult result)
        {
            var resultI = AsyncResultI.check(result, this, _ice_ids_name);
            return ((OutgoingAsyncT<string[]>)resultI.OutgoingAsync).getResult(resultI.wait());
        }

        private void iceI_ice_ids(Dictionary<string, string> context, OutgoingAsyncCompletionCallback completed,
                                  bool synchronous)
        {
            iceCheckAsyncTwowayOnly(_ice_ids_name);
            getOutgoingAsync<string[]>(completed).invoke(_ice_ids_name,
                                                         OperationMode.Nonmutating,
                                                         FormatType.DefaultFormat,
                                                         context,
                                                         synchronous,
                                                         read: (InputStream iss) => { return iss.readStringSeq(); });
        }

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        public string ice_id(OptionalContext context = new OptionalContext())
        {
            try
            {
                return iceI_ice_idAsync(context, null, CancellationToken.None, true).Result;
            }
            catch(AggregateException ex)
            {
                throw ex.InnerException;
            }
        }

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task<string> ice_idAsync(OptionalContext context = new OptionalContext(),
                                        IProgress<bool> progress = null,
                                        CancellationToken cancel = new CancellationToken())
        {
            return iceI_ice_idAsync(context, progress, cancel, false);
        }

        private Task<string>
        iceI_ice_idAsync(OptionalContext context, IProgress<bool> progress, CancellationToken cancel, bool synchronous)
        {
            iceCheckTwowayOnly(_ice_id_name);
            var completed = new OperationTaskCompletionCallback<string>(progress, cancel);
            iceI_ice_id(context, completed, synchronous);
            return completed.Task;
        }

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        public AsyncResult begin_ice_id(AsyncCallback callback, object cookie)
        {
            return iceI_begin_ice_id(new OptionalContext(), callback, cookie, false);
        }

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        public AsyncResult
        begin_ice_id(OptionalContext context, AsyncCallback callback, object cookie)
        {
            return iceI_begin_ice_id(context, callback, cookie, false);
        }

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>An asynchronous result object.</returns>
        public AsyncResult<Callback_Object_ice_id>
        begin_ice_id(OptionalContext context = new OptionalContext())
        {
            return iceI_begin_ice_id(context, null, null, false);
        }

        private const string _ice_id_name = "ice_id";

        private AsyncResult<Callback_Object_ice_id>
        iceI_begin_ice_id(Dictionary<string, string> context, AsyncCallback callback, object cookie, bool synchronous)
        {
            iceCheckAsyncTwowayOnly(_ice_id_name);
            var completed = new OperationAsyncResultCompletionCallback<Callback_Object_ice_id, string>(
                (Callback_Object_ice_id cb, string result) =>
                {
                    if(cb != null)
                    {
                        cb.Invoke(result);
                    }
                }, this, _ice_id_name, cookie, callback);
            iceI_ice_id(context, completed, synchronous);
            return completed;
        }

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="result">The asynchronous result object returned by <code>begin_ice_id</code>.</param>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        public string end_ice_id(AsyncResult result)
        {
            var resultI = AsyncResultI.check(result, this, _ice_id_name);
            return ((OutgoingAsyncT<string>)resultI.OutgoingAsync).getResult(resultI.wait());
        }

        private void iceI_ice_id(Dictionary<string, string> context,
                                 OutgoingAsyncCompletionCallback completed,
                                 bool synchronous)
        {
            getOutgoingAsync<string>(completed).invoke(_ice_id_name,
                                                       OperationMode.Nonmutating,
                                                       FormatType.DefaultFormat,
                                                       context,
                                                       synchronous,
                                                       read: (InputStream iss) => { return iss.readString(); });
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
        public bool ice_invoke(string operation,
                               OperationMode mode,
                               byte[] inEncaps,
                               out byte[] outEncaps,
                               OptionalContext context = new OptionalContext())
        {
            try
            {
                var result = iceI_ice_invokeAsync(operation, mode, inEncaps, context, null, CancellationToken.None, true).Result;
                outEncaps = result.outEncaps;
                return result.returnValue;
            }
            catch(AggregateException ex)
            {
                throw ex.InnerException;
            }
        }

        /// <summary>
        /// Invokes an operation dynamically.
        /// </summary>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task<Object_Ice_invokeResult>
        ice_invokeAsync(string operation,
                        OperationMode mode,
                        byte[] inEncaps,
                        OptionalContext context = new OptionalContext(),
                        IProgress<bool> progress = null,
                        CancellationToken cancel = new CancellationToken())
        {
            return iceI_ice_invokeAsync(operation, mode, inEncaps, context, progress, cancel, false);
        }

        private Task<Object_Ice_invokeResult>
        iceI_ice_invokeAsync(string operation,
                             OperationMode mode,
                             byte[] inEncaps,
                             OptionalContext context,
                             IProgress<bool> progress,
                             CancellationToken cancel,
                             bool synchronous)
        {
            var completed = new InvokeTaskCompletionCallback(progress, cancel);
            iceI_ice_invoke(operation, mode, inEncaps, context, completed, synchronous);
            return completed.Task;
        }

        /// <summary>
        /// Invokes an operation dynamically.
        /// </summary>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        public AsyncResult
        begin_ice_invoke(string operation,
                         OperationMode mode,
                         byte[] inEncaps,
                         AsyncCallback callback,
                         object cookie)
        {
            return iceI_begin_ice_invoke(operation, mode, inEncaps, new OptionalContext(), callback, cookie, false);
        }

        /// <summary>
        /// Invokes an operation dynamically.
        /// </summary>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="callback">A callback to be invoked when the invocation completes.</param>
        /// <param name="cookie">Application-specific data to be stored in the result.</param>
        /// <returns>An asynchronous result object.</returns>
        public AsyncResult
        begin_ice_invoke(string operation,
                         OperationMode mode,
                         byte[] inEncaps,
                         OptionalContext context,
                         AsyncCallback callback,
                         object cookie)
        {
            return iceI_begin_ice_invoke(operation, mode, inEncaps, context, callback, cookie, false);
        }

        /// <summary>
        /// Invokes an operation dynamically.
        /// </summary>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>An asynchronous result object.</returns>
        public AsyncResult<Callback_Object_ice_invoke>
        begin_ice_invoke(string operation,
                         OperationMode mode,
                         byte[] inEncaps,
                         OptionalContext context = new OptionalContext())
        {
            return iceI_begin_ice_invoke(operation, mode, inEncaps, context, null, null, false);
        }

        private const string _ice_invoke_name = "ice_invoke";

        /// <summary>
        /// Completes a dynamic invocation.
        /// </summary>
        /// <param name="outEncaps">The encoded out parameters or user exception.</param>
        /// <param name="result">The asynchronous result object returned by <code>begin_ice_invoke</code>.</param>
        /// <returns>If the operation completed successfully, the return value
        /// is true. If the operation raises a user exception,
        /// the return value is false; in this case, outEncaps
        /// contains the encoded user exception. If the operation raises a run-time exception,
        /// it throws it directly.</returns>
        public bool end_ice_invoke(out byte[] outEncaps, AsyncResult result)
        {
            var resultI = AsyncResultI.check(result, this, _ice_invoke_name);
            var r = ((InvokeOutgoingAsyncT)resultI.OutgoingAsync).getResult(resultI.wait());
            outEncaps = r.outEncaps;
            return r.returnValue;
        }

        private AsyncResult<Callback_Object_ice_invoke>
        iceI_begin_ice_invoke(string operation,
                              OperationMode mode,
                              byte[] inEncaps,
                              Dictionary<string, string> context,
                              AsyncCallback callback,
                              object cookie,
                              bool synchronous)
        {
            var completed = new InvokeAsyncResultCompletionCallback(this, _ice_invoke_name, cookie, callback);
            iceI_ice_invoke(operation, mode, inEncaps, context, completed, synchronous);
            return completed;
        }

        private void iceI_ice_invoke(string operation,
                                     OperationMode mode,
                                     byte[] inEncaps,
                                     Dictionary<string, string> context,
                                     OutgoingAsyncCompletionCallback completed,
                                     bool synchronous)
        {
            getInvokeOutgoingAsync(completed).invoke(operation, mode, inEncaps, context, synchronous);
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
                var proxy = new ObjectPrxHelperBase();
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
                var proxy = new ObjectPrxHelperBase();
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
                var endpts = new EndpointI[newEndpoints.Length];
                for(int i = 0; i < newEndpoints.Length; ++i)
                {
                    endpts[i] = (EndpointI)newEndpoints[i];
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
                throw new ArgumentException("invalid value passed to ice_locatorCacheTimeout: " + newTimeout);
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
                throw new ArgumentException("invalid value passed to ice_invocationTimeout: " + newTimeout);
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
        public ObjectPrx ice_encodingVersion(EncodingVersion e)
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
        public EncodingVersion ice_getEncodingVersion()
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
        public RouterPrx ice_getRouter()
        {
            RouterInfo ri = _reference.getRouterInfo();
            return ri != null ? ri.getRouter() : null;
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the router.
        /// </summary>
        /// <param name="router">The router for the new proxy.</param>
        /// <returns>The new proxy with the specified router.</returns>
        public ObjectPrx ice_router(RouterPrx router)
        {
            Reference @ref = _reference.changeRouter(router);
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
        public LocatorPrx ice_getLocator()
        {
            var li = _reference.getLocatorInfo();
            return li != null ? li.getLocator() : null;
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for the locator.
        /// </summary>
        /// <param name="locator">The locator for the new proxy.</param>
        /// <returns>The new proxy with the specified locator.</returns>
        public ObjectPrx ice_locator(LocatorPrx locator)
        {
            var @ref = _reference.changeLocator(locator);
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
            if(_reference.getMode() == Reference.Mode.ModeTwoway)
            {
                return this;
            }
            else
            {
                return newInstance(_reference.changeMode(Reference.Mode.ModeTwoway));
            }
        }

        /// <summary>
        /// Returns whether this proxy uses twoway invocations.
        /// </summary>
        /// <returns>True if this proxy uses twoway invocations; false, otherwise.</returns>
        public bool ice_isTwoway()
        {
            return _reference.getMode() == Reference.Mode.ModeTwoway;
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, but uses oneway invocations.
        /// </summary>
        /// <returns>A new proxy that uses oneway invocations.</returns>
        public ObjectPrx ice_oneway()
        {
            if(_reference.getMode() == Reference.Mode.ModeOneway)
            {
                return this;
            }
            else
            {
                return newInstance(_reference.changeMode(Reference.Mode.ModeOneway));
            }
        }

        /// <summary>
        /// Returns whether this proxy uses oneway invocations.
        /// </summary>
        /// <returns>True if this proxy uses oneway invocations; false, otherwise.</returns>
        public bool ice_isOneway()
        {
            return _reference.getMode() == Reference.Mode.ModeOneway;
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, but uses batch oneway invocations.
        /// </summary>
        /// <returns>A new proxy that uses batch oneway invocations.</returns>
        public ObjectPrx ice_batchOneway()
        {
            if(_reference.getMode() == Reference.Mode.ModeBatchOneway)
            {
                return this;
            }
            else
            {
                return newInstance(_reference.changeMode(Reference.Mode.ModeBatchOneway));
            }
        }

        /// <summary>
        /// Returns whether this proxy uses batch oneway invocations.
        /// </summary>
        /// <returns>True if this proxy uses batch oneway invocations; false, otherwise.</returns>
        public bool ice_isBatchOneway()
        {
            return _reference.getMode() == Reference.Mode.ModeBatchOneway;
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, but uses datagram invocations.
        /// </summary>
        /// <returns>A new proxy that uses datagram invocations.</returns>
        public ObjectPrx ice_datagram()
        {
            if(_reference.getMode() == Reference.Mode.ModeDatagram)
            {
                return this;
            }
            else
            {
                return newInstance(_reference.changeMode(Reference.Mode.ModeDatagram));
            }
        }

        /// <summary>
        /// Returns whether this proxy uses datagram invocations.
        /// </summary>
        /// <returns>True if this proxy uses datagram invocations; false, otherwise.</returns>
        public bool ice_isDatagram()
        {
            return _reference.getMode() == Reference.Mode.ModeDatagram;
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, but uses batch datagram invocations.
        /// </summary>
        /// <returns>A new proxy that uses batch datagram invocations.</returns>
        public ObjectPrx ice_batchDatagram()
        {
            if(_reference.getMode() == Reference.Mode.ModeBatchDatagram)
            {
                return this;
            }
            else
            {
                return newInstance(_reference.changeMode(Reference.Mode.ModeBatchDatagram));
            }
        }

        /// <summary>
        /// Returns whether this proxy uses batch datagram invocations.
        /// </summary>
        /// <returns>True if this proxy uses batch datagram invocations; false, otherwise.</returns>
        public bool ice_isBatchDatagram()
        {
            return _reference.getMode() == Reference.Mode.ModeBatchDatagram;
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for compression.
        /// </summary>
        /// <param name="co">True enables compression for the new proxy; false disables compression.</param>
        /// <returns>A new proxy with the specified compression setting.</returns>
        public ObjectPrx ice_compress(bool co)
        {
            var @ref = _reference.changeCompress(co);
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
        /// Obtains the compression override setting of this proxy.
        /// </summary>
        /// <returns>The compression override setting. If no optional value is present, no override is
        /// set. Otherwise, true if compression is enabled, false otherwise.</returns>
        public Ice.Optional<bool> ice_getCompress()
        {
            return _reference.getCompress();
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
                throw new ArgumentException("invalid value passed to ice_timeout: " + t);
            }
            var @ref = _reference.changeTimeout(t);
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
        /// Obtains the timeout override of this proxy.
        /// </summary>
        /// <returns>The timeout override. If no optional value is present, no override is set. Otherwise,
        /// returns the timeout override value.</returns>
        public Ice.Optional<int> ice_getTimeout()
        {
            return _reference.getTimeout();
        }

        /// <summary>
        /// Creates a new proxy that is identical to this proxy, except for its connection ID.
        /// </summary>
        /// <param name="connectionId">The connection ID for the new proxy. An empty string removes the
        /// connection ID.</param>
        /// <returns>A new proxy with the specified connection ID.</returns>
        public ObjectPrx ice_connectionId(string connectionId)
        {
            var @ref = _reference.changeConnectionId(connectionId);
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
        /// Returns a proxy that is identical to this proxy, except it's a fixed proxy bound
        /// the given connection.
        /// </summary>
        /// <param name="connection">The fixed proxy connection.</param>
        /// <returns>A fixed proxy bound to the given connection.</returns>
        public ObjectPrx ice_fixed(Ice.Connection connection)
        {
            if(connection == null)
            {
                throw new ArgumentException("invalid null connection passed to ice_fixed");
            }
            if(!(connection is Ice.ConnectionI))
            {
                throw new ArgumentException("invalid connection passed to ice_fixed");
            }
            var @ref = _reference.changeConnection((Ice.ConnectionI)connection);
            if(@ref.Equals(_reference))
            {
                return this;
            }
            else
            {
                return newInstance(@ref);
            }
        }

        private class ProxyGetConnectionAsyncCallback : ProxyAsyncResultCompletionCallback<Callback_Object_ice_getConnection>
        {
            public ProxyGetConnectionAsyncCallback(ObjectPrxHelperBase proxy, string operation, object cookie,
                                                   AsyncCallback cb) :
                base(proxy, operation, cookie, cb)
            {
            }

            protected override AsyncCallback getCompletedCallback()
            {
                return (AsyncResult result) =>
                {
                    try
                    {
                        result.throwLocalException();
                        if(responseCallback_ != null)
                        {
                            responseCallback_.Invoke(((ProxyGetConnection)OutgoingAsync).getConnection());
                        }

                    }
                    catch(Exception ex)
                    {
                        if(exceptionCallback_ != null)
                        {
                            exceptionCallback_.Invoke(ex);
                        }
                    }
                };
            }
        }

        public class GetConnectionTaskCompletionCallback : TaskCompletionCallback<Connection>
        {
            public GetConnectionTaskCompletionCallback(ObjectPrx proxy,
                                                       IProgress<bool> progress = null,
                                                       CancellationToken cancellationToken = new CancellationToken()) :
                base(progress, cancellationToken)
            {
                _proxy = proxy;
            }

            public override void handleInvokeResponse(bool ok, OutgoingAsyncBase og)
            {
                SetResult(((ProxyGetConnection)og).getConnection());
            }

            private ObjectPrx _proxy;
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
            try
            {
                var completed = new GetConnectionTaskCompletionCallback(this);
                iceI_ice_getConnection(completed, true);
                return completed.Task.Result;
            }
            catch(AggregateException ex)
            {
                throw ex.InnerException;
            }
        }

        public Task<Connection> ice_getConnectionAsync(IProgress<bool> progress = null,
                                                       CancellationToken cancel = new CancellationToken())
        {
            var completed = new GetConnectionTaskCompletionCallback(this, progress, cancel);
            iceI_ice_getConnection(completed, false);
            return completed.Task;
        }

        public AsyncResult<Callback_Object_ice_getConnection> begin_ice_getConnection()
        {
            var completed = new ProxyGetConnectionAsyncCallback(this, _ice_getConnection_name, null, null);
            iceI_ice_getConnection(completed, false);
            return completed;
        }

        private const string _ice_getConnection_name = "ice_getConnection";

        public AsyncResult begin_ice_getConnection(AsyncCallback cb, object cookie)
        {
            var completed = new ProxyGetConnectionAsyncCallback(this, _ice_getConnection_name, cookie, cb);
            iceI_ice_getConnection(completed, false);
            return completed;
        }

        public Connection end_ice_getConnection(AsyncResult r)
        {
            var resultI = AsyncResultI.check(r, this, _ice_getConnection_name);
            resultI.wait();
            return ((ProxyGetConnection)resultI.OutgoingAsync).getConnection();
        }

        private void iceI_ice_getConnection(OutgoingAsyncCompletionCallback completed, bool synchronous)
        {
            var outgoing = new ProxyGetConnection(this, completed);
            try
            {
                outgoing.invoke(_ice_getConnection_name, synchronous);
            }
            catch(Exception ex)
            {
                outgoing.abort(ex);
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
            RequestHandler handler;
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

        private class ProxyFlushBatchRequestsAsyncCallback : AsyncResultCompletionCallback
        {
            public ProxyFlushBatchRequestsAsyncCallback(ObjectPrx proxy,
                                                        string operation,
                                                        object cookie,
                                                        AsyncCallback callback) :
                base(proxy.ice_getCommunicator(), ((ObjectPrxHelperBase)proxy).iceReference().getInstance(),
                     operation, cookie, callback)
            {
                _proxy = proxy;
            }

            public override ObjectPrx getProxy()
            {
                return _proxy;
            }

            protected override AsyncCallback getCompletedCallback()
            {
                return (AsyncResult result) =>
                {
                    try
                    {
                        result.throwLocalException();
                    }
                    catch(Exception ex)
                    {
                        if(exceptionCallback_ != null)
                        {
                            exceptionCallback_.Invoke(ex);
                        }
                    }
                };
            }

            private ObjectPrx _proxy;
        }

        /// <summary>
        /// Flushes any pending batched requests for this communicator. The call blocks until the flush is complete.
        /// </summary>
        public void ice_flushBatchRequests()
        {
            try
            {
                var completed = new FlushBatchTaskCompletionCallback();
                iceI_ice_flushBatchRequests(completed, true);
                completed.Task.Wait();
            }
            catch(AggregateException ex)
            {
                throw ex.InnerException;
            }
        }

        internal const string _ice_flushBatchRequests_name = "ice_flushBatchRequests";

        public Task ice_flushBatchRequestsAsync(IProgress<bool> progress = null,
                                                CancellationToken cancel = new CancellationToken())
        {
            var completed = new FlushBatchTaskCompletionCallback(progress, cancel);
            iceI_ice_flushBatchRequests(completed, false);
            return completed.Task;
        }

        public AsyncResult begin_ice_flushBatchRequests(AsyncCallback cb = null, object cookie = null)
        {
            var completed = new ProxyFlushBatchRequestsAsyncCallback(this, _ice_flushBatchRequests_name, cookie, cb);
            iceI_ice_flushBatchRequests(completed, false);
            return completed;
        }

        public void end_ice_flushBatchRequests(AsyncResult r)
        {
            var resultI = AsyncResultI.check(r, this, _ice_flushBatchRequests_name);
            resultI.wait();
        }

        private void iceI_ice_flushBatchRequests(OutgoingAsyncCompletionCallback completed, bool synchronous)
        {
            var outgoing = new ProxyFlushBatchAsync(this, completed);
            try
            {
                outgoing.invoke(_ice_flushBatchRequests_name, synchronous);
            }
            catch(Exception ex)
            {
                outgoing.abort(ex);
            }
        }

        public System.Threading.Tasks.TaskScheduler ice_scheduler()
        {
            return _reference.getThreadPool();
        }

        /// <summary>
        /// Returns whether this proxy equals the passed object. Two proxies are equal if they are equal in all
        /// respects, that is, if their object identity, endpoints timeout settings, and so on are all equal.
        /// </summary>
        /// <param name="r">The object to compare this proxy with.</param>
        /// <returns>True if this proxy is equal to r; false, otherwise.</returns>
        public override bool Equals(object r)
        {
            var rhs = r as ObjectPrxHelperBase;
            return ReferenceEquals(rhs, null) ? false : _reference.Equals(rhs._reference);
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
            return ReferenceEquals(lhs, null) ? ReferenceEquals(rhs, null) : lhs.Equals(rhs);
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

        [EditorBrowsable(EditorBrowsableState.Never)]
        public void iceWrite(OutputStream os)
        {
            _reference.getIdentity().ice_writeMembers(os);
            _reference.streamWrite(os);
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public Reference iceReference()
        {
            return _reference;
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public void iceCopyFrom(ObjectPrx from)
        {
            lock(from)
            {
                var h = (ObjectPrxHelperBase)from;
                _reference = h._reference;
                _requestHandler = h._requestHandler;
            }
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public int iceHandleException(Exception ex, RequestHandler handler, OperationMode mode, bool sent,
                                     ref int cnt)
        {
            iceUpdateRequestHandler(handler, null); // Clear the request handler

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

        [EditorBrowsable(EditorBrowsableState.Never)]
        public void iceCheckTwowayOnly(string name)
        {
            //
            // No mutex lock necessary, there is nothing mutable in this
            // operation.
            //

            if(!ice_isTwoway())
            {
                throw new TwowayOnlyException(name);
            }
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public void iceCheckAsyncTwowayOnly(string name)
        {
            //
            // No mutex lock necessary, there is nothing mutable in this
            // operation.
            //

            if(!ice_isTwoway())
            {
                throw new ArgumentException("`" + name + "' can only be called with a twoway proxy");
            }
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public RequestHandler iceGetRequestHandler()
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

        [EditorBrowsable(EditorBrowsableState.Never)]
        public BatchRequestQueue
        iceGetBatchRequestQueue()
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

        [EditorBrowsable(EditorBrowsableState.Never)]
        public RequestHandler
        iceSetRequestHandler(RequestHandler handler)
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

        [EditorBrowsable(EditorBrowsableState.Never)]
        public void iceUpdateRequestHandler(RequestHandler previous, RequestHandler handler)
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

        protected OutgoingAsyncT<T>
        getOutgoingAsync<T>(OutgoingAsyncCompletionCallback completed)
        {
            bool haveEntry = false;
            InputStream iss = null;
            OutputStream os = null;

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
                return new OutgoingAsyncT<T>(this, completed);
            }
            else
            {
                return new OutgoingAsyncT<T>(this, completed, os, iss);
            }
        }

        private class InvokeOutgoingAsyncT : OutgoingAsync
        {
            public InvokeOutgoingAsyncT(ObjectPrxHelperBase prx,
                                        OutgoingAsyncCompletionCallback completionCallback,
                                        OutputStream os = null,
                                        InputStream iss = null) : base(prx, completionCallback, os, iss)
            {
            }

            public void invoke(string operation, OperationMode mode, byte[] inParams,
                               Dictionary<string, string> context, bool synchronous)
            {
                try
                {
                    prepare(operation, mode, context);
                    if(inParams == null || inParams.Length == 0)
                    {
                        os_.writeEmptyEncapsulation(encoding_);
                    }
                    else
                    {
                        os_.writeEncapsulation(inParams);
                    }
                    invoke(operation, synchronous);
                }
                catch(Exception ex)
                {
                    abort(ex);
                }
            }

            public Object_Ice_invokeResult
            getResult(bool ok)
            {
                try
                {
                    var ret = new Object_Ice_invokeResult();
                    EncodingVersion encoding;
                    if(proxy_.iceReference().getMode() == Reference.Mode.ModeTwoway)
                    {
                        ret.outEncaps = is_.readEncapsulation(out encoding);
                    }
                    else
                    {
                        ret.outEncaps = null;
                    }
                    ret.returnValue = ok;
                    return ret;
                }
                finally
                {
                    cacheMessageBuffers();
                }
            }
        }

        public class InvokeAsyncResultCompletionCallback : ProxyAsyncResultCompletionCallback<Callback_Object_ice_invoke>
        {
            public InvokeAsyncResultCompletionCallback(ObjectPrxHelperBase proxy,
                                                       string operation,
                                                       object cookie,
                                                       AsyncCallback callback) :
                base(proxy, operation, cookie, callback)
            {
            }

            override protected AsyncCallback getCompletedCallback()
            {
                return (AsyncResult r) =>
                {
                    Debug.Assert(r == this);
                    try
                    {
                        Object_Ice_invokeResult result = ((InvokeOutgoingAsyncT)outgoing_).getResult(wait());
                        try
                        {
                            if(responseCallback_ != null)
                            {
                                responseCallback_.Invoke(result.returnValue, result.outEncaps);
                            }
                        }
                        catch(Exception ex)
                        {
                            throw new AggregateException(ex);
                        }
                    }
                    catch(Exception ex)
                    {
                        if(exceptionCallback_ != null)
                        {
                            exceptionCallback_.Invoke(ex);
                        }
                    }
                };
            }
        };

        private class InvokeTaskCompletionCallback : TaskCompletionCallback<Object_Ice_invokeResult>
        {
            public InvokeTaskCompletionCallback(IProgress<bool> progress, CancellationToken cancellationToken) :
                base(progress, cancellationToken)
            {
            }

            public override void handleInvokeSent(bool sentSynchronously, bool done, bool alreadySent,
                                                  OutgoingAsyncBase og)
            {
                if(progress_ != null && !alreadySent)
                {
                    progress_.Report(sentSynchronously);
                }
                if(done)
                {
                    SetResult(new Object_Ice_invokeResult(true, null));
                }
            }

            public override void handleInvokeResponse(bool ok, OutgoingAsyncBase og)
            {
                SetResult(((InvokeOutgoingAsyncT)og).getResult(ok));
            }
        }

        private InvokeOutgoingAsyncT
        getInvokeOutgoingAsync(OutgoingAsyncCompletionCallback completed)
        {
            bool haveEntry = false;
            InputStream iss = null;
            OutputStream os = null;

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
                return new InvokeOutgoingAsyncT(this, completed);
            }
            else
            {
                return new InvokeOutgoingAsyncT(this, completed, os, iss);
            }
        }

        /// <summary>
        /// Only for internal use by OutgoingAsync
        /// </summary>
        /// <param name="iss"></param>
        /// <param name="os"></param>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void
        cacheMessageBuffers(InputStream iss, OutputStream os)
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

        /// <summary>
        /// Only for internal use by ProxyFactory
        /// </summary>
        /// <param name="ref"></param>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void setup(Reference @ref)
        {
            //
            // No need to synchronize, as this operation is only called
            // upon initial initialization.
            //
            Debug.Assert(_reference == null);
            Debug.Assert(_requestHandler == null);

            _reference = @ref;
        }

        private ObjectPrxHelperBase newInstance(Reference @ref)
        {
            var proxy = (ObjectPrxHelperBase)Activator.CreateInstance(GetType());
            proxy.setup(@ref);
            return proxy;
        }

        private Reference _reference;
        private RequestHandler _requestHandler;
        private BatchRequestQueue _batchRequestQueue;
        private struct StreamCacheEntry
        {
            public InputStream iss;
            public OutputStream os;
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
                    var bb = b.ice_facet(f);
                    var ok = bb.ice_isA("::Ice::Object");
                    Debug.Assert(ok);
                    ObjectPrxHelper h = new ObjectPrxHelper();
                    h.iceCopyFrom(bb);
                    d = h;
                }
                catch(FacetNotExistException)
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
                    var bb = b.ice_facet(f);
                    var ok = bb.ice_isA("::Ice::Object", ctx);
                    Debug.Assert(ok);
                    ObjectPrxHelper h = new ObjectPrxHelper();
                    h.iceCopyFrom(bb);
                    d = h;
                }
                catch(FacetNotExistException)
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
                var bb = b.ice_facet(f);
                var h = new ObjectPrxHelper();
                h.iceCopyFrom(bb);
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
            return ObjectImpl.ice_staticId();
        }
    }
}
