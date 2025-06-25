// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.time.Duration;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Executor;

/** Base interface of all object proxies. */
public interface ObjectPrx {
    /**
     * Gets the communicator that created this proxy.
     *
     * @return The communicator that created this proxy.
     */
    Communicator ice_getCommunicator();

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param id The type ID of the Slice interface to test against.
     * @return {@code true} if the target object has the interface specified by {@code id}
     *     or derives from the interface specified by {@code id}.
     */
    boolean ice_isA(String id);

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param id The type ID of the Slice interface to test against.
     * @param context The context map for the invocation.
     * @return {@code true} if the target object has the interface specified by {@code id}
     *     or derives from the interface specified by {@code id}.
     */
    boolean ice_isA(String id, Map<String, String> context);

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param id The type ID of the Slice interface to test against.
     * @return A future for the completion of the request.
     */
    CompletableFuture<Boolean> ice_isAAsync(String id);

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param id The type ID of the Slice interface to test against.
     * @param context The context map for the invocation.
     * @return A future for the completion of the request.
     */
    CompletableFuture<Boolean> ice_isAAsync(
            String id, Map<String, String> context);

    /** Tests whether the target object of this proxy can be reached. */
    void ice_ping();

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param context The context map for the invocation.
     */
    void ice_ping(Map<String, String> context);

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @return A future for the completion of the request.
     */
    CompletableFuture<Void> ice_pingAsync();

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param context The context map for the invocation.
     * @return A future for the completion of the request.
     */
    CompletableFuture<Void> ice_pingAsync(
            Map<String, String> context);

    /**
     * Gets the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @return The Slice type IDs of the interfaces supported by the target object, in alphabetical
     *     order.
     */
    String[] ice_ids();

    /**
     * Gets the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @param context The context map for the invocation.
     * @return The Slice type IDs of the interfaces supported by the target object, in alphabetical
     *     order.
     */
    String[] ice_ids(Map<String, String> context);

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @return A future for the completion of the request.
     */
    CompletableFuture<String[]> ice_idsAsync();

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @param context The context map for the invocation.
     * @return A future for the completion of the request.
     */
    CompletableFuture<String[]> ice_idsAsync(
            Map<String, String> context);

    /**
     * Gets the Slice type ID of the most-derived interface supported by the target object of
     * this proxy.
     *
     * @return The Slice type ID of the most-derived interface.
     */
    String ice_id();

    /**
     * Gets the Slice type ID of the most-derived interface supported by the target object of
     * this proxy.
     *
     * @param context The context map for the invocation.
     * @return The Slice type ID of the most-derived interface.
     */
    String ice_id(Map<String, String> context);

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of
     * this proxy.
     *
     * @return A future for the completion of the request.
     */
    CompletableFuture<String> ice_idAsync();

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of
     * this proxy.
     *
     * @param context The context map for the invocation.
     * @return A future for the completion of the request.
     */
    CompletableFuture<String> ice_idAsync(
            Map<String, String> context);

    /**
     * Invokes an operation dynamically.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * @return The results of the invocation.
     * @see Blobject
     * @see OperationMode
     */
    Object.Ice_invokeResult ice_invoke(
            String operation, OperationMode mode, byte[] inParams);

    /**
     * Invokes an operation dynamically.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * @param context The context map for the invocation.
     * @return The results of the invocation.
     * @see Blobject
     * @see OperationMode
     */
    Object.Ice_invokeResult ice_invoke(
            String operation,
            OperationMode mode,
            byte[] inParams,
            Map<String, String> context);

    /**
     * Invokes an operation asynchronously.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * @return A future for the completion of the request.
     * @see Blobject
     * @see OperationMode
     */
    CompletableFuture<Object.Ice_invokeResult> ice_invokeAsync(
            String operation, OperationMode mode, byte[] inParams);

    /**
     * Invokes an operation asynchronously.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * @param context The context map for the invocation.
     * @return A future for the completion of the request.
     * @see Blobject
     * @see OperationMode
     */
    CompletableFuture<Object.Ice_invokeResult> ice_invokeAsync(
            String operation,
            OperationMode mode,
            byte[] inParams,
            Map<String, String> context);

    /**
     * Gets the identity embedded in this proxy.
     *
     * @return The identity of the target object.
     */
    Identity ice_getIdentity();

    /**
     * Creates a proxy that is identical to this proxy, except for the identity.
     *
     * @param newIdentity The identity for the new proxy.
     * @return A proxy with the new identity.
     */
    ObjectPrx ice_identity(Identity newIdentity);

    /**
     * Gets the per-proxy context for this proxy.
     *
     * @return The per-proxy context. If the proxy does not have a per-proxy (implicit) context, the
     *     return value is null.
     */
    Map<String, String> ice_getContext();

    /**
     * Creates a proxy that is identical to this proxy, except for the per-proxy context.
     *
     * @param newContext The context for the new proxy.
     * @return A proxy with the new per-proxy context.
     */
    ObjectPrx ice_context(Map<String, String> newContext);

    /**
     * Gets the facet for this proxy.
     *
     * @return The facet for this proxy. If the proxy uses the default facet, the return value is
     *     the empty string.
     */
    String ice_getFacet();

    /**
     * Creates a proxy that is identical to this proxy, except for the facet.
     *
     * @param newFacet The facet for the new proxy.
     * @return A proxy with the new facet.
     */
    ObjectPrx ice_facet(String newFacet);

    /**
     * Gets the adapter ID for this proxy.
     *
     * @return The adapter ID. If the proxy does not have an adapter ID, the return value is the
     *     empty string.
     */
    String ice_getAdapterId();

    /**
     * Creates a proxy that is identical to this proxy, except for the adapter ID.
     *
     * @param newAdapterId The adapter ID for the new proxy.
     * @return A proxy with the new adapter ID.
     */
    ObjectPrx ice_adapterId(String newAdapterId);

    /**
     * Gets the endpoints used by this proxy.
     *
     * @return The endpoints used by this proxy.
     * @see Endpoint
     */
    Endpoint[] ice_getEndpoints();

    /**
     * Creates a proxy that is identical to this proxy, except for the endpoints.
     *
     * @param newEndpoints The endpoints for the new proxy.
     * @return A proxy with the new endpoints.
     */
    ObjectPrx ice_endpoints(Endpoint[] newEndpoints);

    /**
     * Gets the locator cache timeout of this proxy.
     *
     * @return The locator cache timeout value.
     * @see Locator
     */
    Duration ice_getLocatorCacheTimeout();

    /**
     * Gets the invocation timeout of this proxy.
     *
     * @return The invocation timeout value.
     */
    Duration ice_getInvocationTimeout();

    /**
     * Gets the connection id of this proxy.
     *
     * @return The connection id.
     */
    String ice_getConnectionId();

    /**
     * Creates a proxy that is identical to this proxy, except it's a fixed proxy bound to the given
     * connection.
     *
     * @param connection The fixed proxy connection.
     * @return A fixed proxy bound to the given connection.
     */
    ObjectPrx ice_fixed(Connection connection);

    /**
     * Determines whether this proxy is a fixed proxy.
     *
     * @return {@code true} if this is a fixed proxy, {@code false} otherwise.
     */
    boolean ice_isFixed();

    /**
     * Creates a proxy that is identical to this proxy, except for the locator cache timeout.
     *
     * @param newTimeout The new locator cache timeout (in seconds).
     * @return A proxy with the new timeout.
     * @see Locator
     */
    ObjectPrx ice_locatorCacheTimeout(int newTimeout);

    /**
     * Returns a proxy that is identical to this proxy, except for the locator cache timeout.
     *
     * @param newTimeout The new locator cache timeout.
     * @return The proxy with the new timeout.
     * @see Locator
     */
    ObjectPrx ice_locatorCacheTimeout(Duration newTimeout);

    /**
     * Returns a proxy that is identical to this proxy, except for the invocation timeout.
     *
     * @param newTimeout The new invocation timeout (in milliseconds).
     * @return The proxy with the new timeout.
     */
    ObjectPrx ice_invocationTimeout(int newTimeout);

    /**
     * Returns a proxy that is identical to this proxy, except for the invocation timeout.
     *
     * @param newTimeout The new invocation timeout.
     * @return The proxy with the new timeout.
     */
    ObjectPrx ice_invocationTimeout(Duration newTimeout);

    /**
     * Determines whether this proxy caches connections.
     *
     * @return {@code true} if this proxy caches connections, {@code false} otherwise.
     */
    boolean ice_isConnectionCached();

    /**
     * Returns a proxy that is identical to this proxy, except for connection caching.
     *
     * @param newCache <code>true</code> if the new proxy should cache connections; <code>false
     *     </code> otherwise.
     * @return The proxy with the specified caching policy.
     */
    ObjectPrx ice_connectionCached(boolean newCache);

    /**
     * Gets the endpoint selection policy for this proxy (randomly or ordered).
     *
     * @return The endpoint selection policy.
     * @see EndpointSelectionType
     */
    EndpointSelectionType ice_getEndpointSelection();

    /**
     * Returns a proxy that is identical to this proxy, except for the endpoint selection policy.
     *
     * @param newType The new endpoint selection policy.
     * @return The proxy with the specified endpoint selection policy.
     * @see EndpointSelectionType
     */
    ObjectPrx ice_endpointSelection(EndpointSelectionType newType);

    /**
     * Determines whether this proxy uses only secure endpoints.
     *
     * @return {@code true} if this proxy communicates only via secure endpoints, {@code false}
     *     otherwise.
     */
    boolean ice_isSecure();

    /**
     * Returns a proxy that is identical to this proxy, except for how it selects endpoints.
     *
     * @param b If <code>b</code> is <code>true</code>, only endpoints that use a secure transport
     *     are used by the new proxy. If <code>b</code> is false, the returned proxy uses both
     *     secure and insecure endpoints.
     * @return The proxy with the specified selection policy.
     */
    ObjectPrx ice_secure(boolean b);

    /**
     * Returns a proxy that is identical to this proxy, except for the encoding used to marshal
     * parameters.
     *
     * @param e The encoding version to use to marshal request parameters.
     * @return The proxy with the specified encoding version.
     */
    ObjectPrx ice_encodingVersion(EncodingVersion e);

    /**
     * Gets the encoding version used to marshal request parameters.
     *
     * @return The encoding version.
     */
    EncodingVersion ice_getEncodingVersion();

    /**
     * Determines whether this proxy prefers secure endpoints.
     *
     * @return {@code true} if the proxy always attempts to invoke via secure endpoints before
     *     it attempts to use insecure endpoints, {@code false} otherwise.
     */
    boolean ice_isPreferSecure();

    /**
     * Returns a proxy that is identical to this proxy, except for its endpoint selection policy.
     *
     * @param b If <code>b</code> is <code>true</code>, the new proxy will use secure endpoints for
     *     invocations and only use insecure endpoints if an invocation cannot be made via secure
     *     endpoints. If <code>b</code> is <code>false</code>, the proxy prefers insecure endpoints
     *     to secure ones.
     * @return The proxy with the specified selection policy.
     */
    ObjectPrx ice_preferSecure(boolean b);

    /**
     * Gets the router for this proxy.
     *
     * @return The router for the proxy. If no router is configured for the proxy, the return value
     *     is null.
     */
    RouterPrx ice_getRouter();

    /**
     * Returns a proxy that is identical to this proxy, except for the router.
     *
     * @param router The router for the new proxy.
     * @return The proxy with the specified router.
     */
    ObjectPrx ice_router(RouterPrx router);

    /**
     * Gets the locator for this proxy.
     *
     * @return The locator for this proxy. If no locator is configured, the return value is null.
     */
    LocatorPrx ice_getLocator();

    /**
     * Returns a proxy that is identical to this proxy, except for the locator.
     *
     * @param locator The locator for the new proxy.
     * @return The proxy with the specified locator.
     */
    ObjectPrx ice_locator(LocatorPrx locator);

    /**
     * Determines whether this proxy uses collocation optimization.
     *
     * @return {@code true} if the proxy uses collocation optimization, {@code false}
     *     otherwise.
     */
    boolean ice_isCollocationOptimized();

    /**
     * Returns a proxy that is identical to this proxy, except for collocation optimization.
     *
     * @param b <code>true</code> if the new proxy enables collocation optimization; <code>false
     *     </code> otherwise.
     * @return The proxy with the specified collocation optimization.
     */
    ObjectPrx ice_collocationOptimized(boolean b);

    /**
     * Creates a proxy that is identical to this proxy, but uses twoway invocations.
     *
     * @return A proxy that uses twoway invocations.
     */
    ObjectPrx ice_twoway();

    /**
     * Determines whether this proxy uses twoway invocations.
     *
     * @return {@code true} if this proxy uses twoway invocations, {@code false}
     *     otherwise.
     */
    boolean ice_isTwoway();

    /**
     * Creates a proxy that is identical to this proxy, but uses oneway invocations.
     *
     * @return A proxy that uses oneway invocations.
     */
    ObjectPrx ice_oneway();

    /**
     * Determines whether this proxy uses oneway invocations.
     *
     * @return {@code true} if this proxy uses oneway invocations, {@code false}
     *     otherwise.
     */
    boolean ice_isOneway();

    /**
     * Creates a proxy that is identical to this proxy, but uses batch oneway invocations.
     *
     * @return A proxy that uses batch oneway invocations.
     */
    ObjectPrx ice_batchOneway();

    /**
     * Determines whether this proxy uses batch oneway invocations.
     *
     * @return {@code true} if this proxy uses batch oneway invocations, {@code false}
     *     otherwise.
     */
    boolean ice_isBatchOneway();

    /**
     * Creates a proxy that is identical to this proxy, but uses datagram invocations.
     *
     * @return A proxy that uses datagram invocations.
     */
    ObjectPrx ice_datagram();

    /**
     * Determines whether this proxy uses datagram invocations.
     *
     * @return {@code true} if this proxy uses datagram invocations, {@code false}
     *     otherwise.
     */
    boolean ice_isDatagram();

    /**
     * Creates a proxy that is identical to this proxy, but uses batch datagram invocations.
     *
     * @return A proxy that uses batch datagram invocations.
     */
    ObjectPrx ice_batchDatagram();

    /**
     * Determines whether this proxy uses batch datagram invocations.
     *
     * @return {@code true} if this proxy uses batch datagram invocations, {@code false}
     *     otherwise.
     */
    boolean ice_isBatchDatagram();

    /**
     * Returns a proxy that is identical to this proxy, except for its compression setting which
     * overrides the compression setting from the proxy endpoints.
     *
     * @param co <code>true</code> enables compression for the new proxy; <code>false</code>
     *     disables compression.
     * @return A proxy with the specified compression setting.
     */
    ObjectPrx ice_compress(boolean co);

    /**
     * Gets the compression override setting of this proxy.
     *
     * @return The compression override setting. If no optional value is present, no override is
     *     set. Otherwise, true if compression is enabled, false otherwise.
     */
    Optional<Boolean> ice_getCompress();

    /**
     * Returns a proxy that is identical to this proxy, except for its connection ID.
     *
     * @param connectionId The connection ID for the new proxy. An empty string removes the
     *     connection ID.
     * @return A proxy with the specified connection ID.
     */
    ObjectPrx ice_connectionId(String connectionId);

    /**
     * Gets the connection for this proxy. If the proxy does not yet have an established
     * connection, it first attempts to create a connection.
     *
     * @return The {@link Connection} for this proxy.
     * @see Connection
     */
    Connection ice_getConnection();

    /**
     * Gets an executor object that uses the Ice thread pool.
     *
     * @return The Executor object.
     */
    default Executor ice_executor() {
        return _getReference().getThreadPool();
    }

    /**
     * Gets the connection for this proxy asynchronously. The call does not block.
     *
     * @return A future for the completion of the request.
     */
    CompletableFuture<Connection> ice_getConnectionAsync();

    /**
     * Gets the cached {@link Connection} for this proxy. If the proxy does not yet have an
     * established connection, it does not attempt to create a connection.
     *
     * @return The cached {@link Connection} for this proxy (null if the proxy does not
     *     have an established connection).
     * @see Connection
     */
    Connection ice_getCachedConnection();

    /**
     * Flushes any pending batched requests for this proxy. The call blocks until the flush
     * is complete.
     */
    void ice_flushBatchRequests();

    /**
     * Flushes any pending batched requests for this proxy asynchronously. The call does not
     * block.
     *
     * @return A future for the completion of the request.
     */
    CompletableFuture<Void> ice_flushBatchRequestsAsync();

    /**
     * Determines whether this proxy equals the passed object. Two proxies are equal if they are equal
     * in all respects, that is, if their object identity, endpoints timeout settings, and so on are
     * all equal.
     *
     * @param r The object to compare this proxy with.
     * @return {@code true} if this proxy is equal to {@code r}, {@code false}
     *     otherwise.
     */
    @Override
    boolean equals(java.lang.Object r);

    /** The type ID of the target's interface: "::Ice::Object". */
    static final String ice_staticId = "::Ice::Object";

    /**
     * Gets the Slice type ID associated with this type.
     *
     * @return The Slice type ID.
     */
    static String ice_staticId() {
        return ice_staticId;
    }

    /**
     * Creates a new proxy that implements {@link ObjectPrx}.
     *
     * @param communicator The communicator of the new proxy.
     * @param proxyString The string representation of the proxy.
     * @return The new proxy.
     * @throws ParseException Thrown when {@code proxyString} is not a valid proxy string.
     */
    public static ObjectPrx createProxy(Communicator communicator, String proxyString) {
        var ref = communicator.getInstance().referenceFactory().create(proxyString, null);
        if (ref == null) {
            throw new ParseException("Invalid empty proxy string.");
        }
        return new _ObjectPrxI(ref);
    }

    /**
     * Casts a proxy to {@link ObjectPrx}. For user-defined types, this call contacts the server and
     * will throw an Ice run-time exception if the target object does not exist or the server cannot
     * be reached.
     *
     * @param obj The proxy to cast to {@link ObjectPrx}.
     * @return {@code obj}.
     */
    static ObjectPrx checkedCast(ObjectPrx obj) {
        return checkedCast(obj, noExplicitContext);
    }

    /**
     * Casts a proxy to {@link ObjectPrx}. For user-defined types, this call contacts the server and
     * throws an Ice run-time exception if the target object does not exist or the server cannot be
     * reached.
     *
     * @param obj The proxy to cast to {@link ObjectPrx}.
     * @param context The {@code Context} map for the invocation.
     * @return {@code obj}.
     */
    static ObjectPrx checkedCast(ObjectPrx obj, Map<String, String> context) {
        return obj != null && obj.ice_isA(ice_staticId, context) ? obj : null;
    }

    /**
     * Creates a new proxy that is identical to the passed proxy, except for its facet. This call
     * contacts the server and throws an Ice run-time exception if the target object does not exist,
     * the specified facet does not exist, or the server cannot be reached.
     *
     * @param obj The proxy to cast to {@link ObjectPrx}.
     * @param facet The facet for the new proxy.
     * @return The new proxy with the specified facet.
     */
    static ObjectPrx checkedCast(ObjectPrx obj, String facet) {
        return checkedCast(obj, facet, noExplicitContext);
    }

    /**
     * Creates a new proxy that is identical to the passed proxy, except for its facet. This call
     * contacts the server and throws an Ice run-time exception if the target object does not exist,
     * the specified facet does not exist, or the server cannot be reached.
     *
     * @param obj The proxy to cast to {@link ObjectPrx}.
     * @param facet The facet for the new proxy.
     * @param context The <code>Context</code> map for the invocation.
     * @return The new proxy with the specified facet.
     */
    static ObjectPrx checkedCast(
            ObjectPrx obj, String facet, Map<String, String> context) {
        return obj == null ? null : checkedCast(obj.ice_facet(facet), context);
    }

    /**
     * Casts a proxy to {@link ObjectPrx}. This call does not contact the server and always
     * succeeds.
     *
     * @param obj The proxy to cast to {@link ObjectPrx}.
     * @return {@code obj}.
     */
    static ObjectPrx uncheckedCast(ObjectPrx obj) {
        return obj;
    }

    /**
     * Creates a new proxy that is identical to the passed proxy, except for its facet. This call
     * does not contact the server and always succeeds.
     *
     * @param obj The proxy to cast to {@link ObjectPrx}.
     * @param facet The facet for the new proxy.
     * @return The new proxy with the specified facet.
     */
    static ObjectPrx uncheckedCast(ObjectPrx obj, String facet) {
        return obj == null ? null : obj.ice_facet(facet);
    }

    /**
     * Writes a proxy to the stream.
     *
     * @param ostr The destination stream.
     * @param v The proxy to write to the stream.
     */
    static void write(OutputStream ostr, ObjectPrx v) {
        ostr.writeProxy(v);
    }

    /**
     * Reads a proxy from the stream.
     *
     * @param istr The source stream.
     * @return A new proxy or null for a null proxy.
     */
    static ObjectPrx read(InputStream istr) {
        return istr.readProxy();
    }

    /**
     * @hidden
     * @param os -
     */
    void _write(OutputStream os);

    /**
     * @hidden
     * @return -
     */
    Reference _getReference();

    /**
     * Creates a new {@code ObjectPrx} instance. This is a helper method and should not be
     * called directly by user code.
     *
     * @param ref the reference to use for the new proxy
     * @return a new {@code ObjectPrx} instance
     * @hidden
     */
    default ObjectPrx _newInstance(Reference ref) {
        return new _ObjectPrxI(ref);
    }

    /**
     * A special empty context that is indistinguishable from the absence of a context parameter.
     * For example, <code>prx.op(noExplicitContext)</code> is the same as <code>prx.op()</code> and
     * does not override the current implicit context (if any).
     */
    static final Map<String, String> noExplicitContext = new HashMap<>();
}
