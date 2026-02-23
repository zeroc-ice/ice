// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.time.Duration;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Executor;

/** The base interface of all Ice proxies. */
public interface ObjectPrx {
    /**
     * Gets the communicator that created this proxy.
     *
     * @return the communicator that created this proxy
     */
    Communicator ice_getCommunicator();

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param id the type ID of the Slice interface to test against
     * @return {@code true} if the target object implements the Slice interface specified by {@code id}
     *     or implements a derived interface, and {@code false} otherwise.
     */
    boolean ice_isA(String id);

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param id the type ID of the Slice interface to test against
     * @param context the request context
     * @return {@code true} if the target object implements the Slice interface specified by {@code id}
     *     or implements a derived interface, and {@code false} otherwise.
     */
    boolean ice_isA(String id, Map<String, String> context);

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param id the type ID of the Slice interface to test against
     * @return a future that completes with {@code true} if the target object implements the Slice interface specified
     *     by {@code id} or implements a derived interface, and {@code false} otherwise.
     */
    CompletableFuture<Boolean> ice_isAAsync(String id);

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param id the type ID of the Slice interface to test against
     * @param context the request context
     * @return a future that completes with {@code true} if the target object implements the Slice interface specified
     *     by {@code id} or implements a derived interface, and {@code false} otherwise.
     */
    CompletableFuture<Boolean> ice_isAAsync(String id, Map<String, String> context);

    /** Tests whether the target object of this proxy can be reached. */
    void ice_ping();

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param context the request context
     */
    void ice_ping(Map<String, String> context);

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @return a future that completes when the invocation completes
     */
    CompletableFuture<Void> ice_pingAsync();

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param context the request context
     * @return a future that completes when the invocation completes
     */
    CompletableFuture<Void> ice_pingAsync(Map<String, String> context);

    /**
     * Returns the Slice interfaces supported by this object as a list of Slice type IDs.
     *
     * @return the Slice type IDs of the interfaces supported by this object, in alphabetical order
     */
    String[] ice_ids();

    /**
     * Returns the Slice interfaces supported by this object as a list of Slice type IDs.
     *
     * @param context the request context
     * @return the Slice type IDs of the interfaces supported by this object, in alphabetical order
     */
    String[] ice_ids(Map<String, String> context);

    /**
     * Returns the Slice interfaces supported by this object as a list of Slice type IDs.
     *
     * @return a future that completes with the Slice type IDs of the interfaces supported by this object,
     *     in alphabetical order
     */
    CompletableFuture<String[]> ice_idsAsync();

    /**
     * Returns the Slice interfaces supported by this object as a list of Slice type IDs.
     *
     * @param context the request context
     * @return a future that completes with the Slice type IDs of the interfaces supported by this object,
     *     in alphabetical order
     */
    CompletableFuture<String[]> ice_idsAsync(Map<String, String> context);

    /**
     * Returns the type ID of the most-derived Slice interface supported by this object.
     *
     * @return the Slice type ID of the most-derived interface
     */
    String ice_id();

    /**
     * Returns the type ID of the most-derived Slice interface supported by this object.
     *
     * @param context the request context
     * @return the Slice type ID of the most-derived interface
     */
    String ice_id(Map<String, String> context);

    /**
     * Returns the type ID of the most-derived Slice interface supported by this object.
     *
     * @return a future that completes with the type ID of the most-derived Slice interface supported by this object
     */
    CompletableFuture<String> ice_idAsync();

    /**
     * Returns the type ID of the most-derived Slice interface supported by this object.
     *
     * @param context the request context
     * @return a future that completes with the type ID of the most-derived Slice interface supported by this object
     */
    CompletableFuture<String> ice_idAsync(Map<String, String> context);

    /**
     * Invokes an operation.
     *
     * @param operation the name of the operation to invoke
     * @param mode the operation mode (normal or idempotent)
     * @param inParams an encapsulation containing the encoded in-parameters for the operation
     * @return the result of the invocation
     * @see Blobject
     */
    Object.Ice_invokeResult ice_invoke(String operation, OperationMode mode, byte[] inParams);

    /**
     * Invokes an operation.
     *
     * @param operation the name of the operation to invoke
     * @param mode the operation mode (normal or idempotent)
     * @param inParams an encapsulation containing the encoded in-parameters for the operation
     * @param context the request context
     * @return the result of the invocation
     * @see Blobject
     */
    Object.Ice_invokeResult ice_invoke(
            String operation,
            OperationMode mode,
            byte[] inParams,
            Map<String, String> context);

    /**
     * Invokes an operation asynchronously.
     *
     * @param operation the name of the operation to invoke
     * @param mode the operation mode (normal or idempotent)
     * @param inParams an encapsulation containing the encoded in-parameters for the operation
     * @return a future that completes with the result of the invocation
     * @see Blobject
     */
    CompletableFuture<Object.Ice_invokeResult> ice_invokeAsync(String operation, OperationMode mode, byte[] inParams);

    /**
     * Invokes an operation asynchronously.
     *
     * @param operation the name of the operation to invoke
     * @param mode the operation mode (normal or idempotent)
     * @param inParams an encapsulation containing the encoded in-parameters for the operation
     * @param context the request context
     * @return a future that completes with the result of the invocation
     * @see Blobject
     */
    CompletableFuture<Object.Ice_invokeResult> ice_invokeAsync(
            String operation,
            OperationMode mode,
            byte[] inParams,
            Map<String, String> context);

    /**
     * Gets the identity embedded in this proxy.
     *
     * @return the identity of the target object
     */
    Identity ice_getIdentity();

    /**
     * Creates a proxy that is identical to this proxy, except for the identity.
     *
     * @param newIdentity the identity for the new proxy
     * @return a proxy with the new identity
     */
    ObjectPrx ice_identity(Identity newIdentity);

    /**
     * Gets the per-proxy context for this proxy.
     *
     * @return the per-proxy context, or {@code null} if the proxy does not have a per-proxy context.
     */
    Map<String, String> ice_getContext();

    /**
     * Creates a proxy that is identical to this proxy, except for the per-proxy context.
     *
     * @param newContext the context for the new proxy
     * @return a proxy with the new per-proxy context
     */
    ObjectPrx ice_context(Map<String, String> newContext);

    /**
     * Gets the facet for this proxy.
     *
     * @return the facet for this proxy. If the proxy uses the default facet, the return value is the empty string.
     */
    String ice_getFacet();

    /**
     * Creates a proxy that is identical to this proxy, except for the facet.
     *
     * @param newFacet the facet for the new proxy
     * @return a proxy with the new facet
     */
    ObjectPrx ice_facet(String newFacet);

    /**
     * Gets the adapter ID for this proxy.
     *
     * @return the adapter ID. If the proxy does not have an adapter ID, the return value is the empty string.
     */
    String ice_getAdapterId();

    /**
     * Creates a proxy that is identical to this proxy, except for the adapter ID.
     *
     * @param newAdapterId the adapter ID for the new proxy
     * @return a proxy with the new adapter ID
     */
    ObjectPrx ice_adapterId(String newAdapterId);

    /**
     * Gets the endpoints used by this proxy.
     *
     * @return the endpoints used by this proxy
     */
    Endpoint[] ice_getEndpoints();

    /**
     * Creates a proxy that is identical to this proxy, except for the endpoints.
     *
     * @param newEndpoints the endpoints for the new proxy
     * @return a proxy with the new endpoints
     */
    ObjectPrx ice_endpoints(Endpoint[] newEndpoints);

    /**
     * Gets the locator cache timeout of this proxy.
     *
     * @return the locator cache timeout value
     * @see Locator
     */
    Duration ice_getLocatorCacheTimeout();

    /**
     * Gets the invocation timeout of this proxy.
     *
     * @return the invocation timeout value
     */
    Duration ice_getInvocationTimeout();

    /**
     * Gets the connection id of this proxy.
     *
     * @return the connection ID
     */
    String ice_getConnectionId();

    /**
     * Creates a proxy that is identical to this proxy, except it's a fixed proxy bound to the given connection.
     *
     * @param connection the fixed proxy connection
     * @return a fixed proxy bound to the given connection
     */
    ObjectPrx ice_fixed(Connection connection);

    /**
     * Determines whether this proxy is a fixed proxy.
     *
     * @return {@code true} if this is a fixed proxy, {@code false} otherwise
     */
    boolean ice_isFixed();

    /**
     * Creates a proxy that is identical to this proxy, except for the locator cache timeout.
     *
     * @param newTimeout the new locator cache timeout (in seconds)
     * @return a proxy with the new timeout
     * @see Locator
     */
    ObjectPrx ice_locatorCacheTimeout(int newTimeout);

    /**
     * Creates a proxy that is identical to this proxy, except for the locator cache timeout.
     *
     * @param newTimeout the new locator cache timeout
     * @return a proxy with the new timeout
     * @see Locator
     */
    ObjectPrx ice_locatorCacheTimeout(Duration newTimeout);

    /**
     * Creates a proxy that is identical to this proxy, except for the invocation timeout.
     *
     * @param newTimeout the new invocation timeout (in milliseconds)
     * @return a proxy with the new timeout
     */
    ObjectPrx ice_invocationTimeout(int newTimeout);

    /**
     * Creates a proxy that is identical to this proxy, except for the invocation timeout.
     *
     * @param newTimeout the new invocation timeout
     * @return a proxy with the new timeout
     */
    ObjectPrx ice_invocationTimeout(Duration newTimeout);

    /**
     * Determines whether this proxy caches connections.
     *
     * @return {@code true} if this proxy caches connections, {@code false} otherwise
     */
    boolean ice_isConnectionCached();

    /**
     * Creates a proxy that is identical to this proxy, except for connection caching.
     *
     * @param newCache {@code true} if the new proxy should cache connections, {@code false} otherwise
     * @return a proxy with the specified caching policy
     */
    ObjectPrx ice_connectionCached(boolean newCache);

    /**
     * Gets the endpoint selection policy for this proxy (randomly or ordered).
     *
     * @return the endpoint selection policy
     */
    EndpointSelectionType ice_getEndpointSelection();

    /**
     * Creates a proxy that is identical to this proxy, except for the endpoint selection policy.
     *
     * @param newType the new endpoint selection policy
     * @return a proxy with the specified endpoint selection policy
     */
    ObjectPrx ice_endpointSelection(EndpointSelectionType newType);

    /**
     * Creates a proxy that is identical to this proxy, except for the encoding used to marshal parameters.
     *
     * @param e the encoding version to use to marshal request parameters
     * @return a proxy with the specified encoding version
     */
    ObjectPrx ice_encodingVersion(EncodingVersion e);

    /**
     * Gets the encoding version used to marshal request parameters.
     *
     * @return the encoding version
     */
    EncodingVersion ice_getEncodingVersion();

    /**
     * Gets the router for this proxy.
     *
     * @return the router for the proxy. If no router is configured for the proxy, the return value is {@code null}.
     */
    RouterPrx ice_getRouter();

    /**
     * Creates a proxy that is identical to this proxy, except for the router.
     *
     * @param router the router for the new proxy
     * @return a proxy with the specified router
     */
    ObjectPrx ice_router(RouterPrx router);

    /**
     * Gets the locator for this proxy.
     *
     * @return the locator for this proxy. If no locator is configured, the return value is {@code null}.
     */
    LocatorPrx ice_getLocator();

    /**
     * Creates a proxy that is identical to this proxy, except for the locator.
     *
     * @param locator the locator for the new proxy
     * @return a proxy with the specified locator
     */
    ObjectPrx ice_locator(LocatorPrx locator);

    /**
     * Determines whether this proxy uses collocation optimization.
     *
     * @return {@code true} if the proxy uses collocation optimization, {@code false} otherwise
     */
    boolean ice_isCollocationOptimized();

    /**
     * Creates a proxy that is identical to this proxy, except for collocation optimization.
     *
     * @param b {@code true} if the new proxy enables collocation optimization, {@code false} otherwise
     * @return a proxy with the specified collocation optimization
     */
    ObjectPrx ice_collocationOptimized(boolean b);

    /**
     * Creates a proxy that is identical to this proxy, but uses twoway invocations.
     *
     * @return a proxy that uses twoway invocations
     */
    ObjectPrx ice_twoway();

    /**
     * Determines whether this proxy uses twoway invocations.
     *
     * @return {@code true} if this proxy uses twoway invocations, {@code false} otherwise
     */
    boolean ice_isTwoway();

    /**
     * Creates a proxy that is identical to this proxy, but uses oneway invocations.
     *
     * @return a proxy that uses oneway invocations
     */
    ObjectPrx ice_oneway();

    /**
     * Determines whether this proxy uses oneway invocations.
     *
     * @return {@code true} if this proxy uses oneway invocations, {@code false} otherwise
     */
    boolean ice_isOneway();

    /**
     * Creates a proxy that is identical to this proxy, but uses batch oneway invocations.
     *
     * @return a proxy that uses batch oneway invocations
     */
    ObjectPrx ice_batchOneway();

    /**
     * Determines whether this proxy uses batch oneway invocations.
     *
     * @return {@code true} if this proxy uses batch oneway invocations, {@code false} otherwise
     */
    boolean ice_isBatchOneway();

    /**
     * Creates a proxy that is identical to this proxy, but uses datagram invocations.
     *
     * @return a proxy that uses datagram invocations
     */
    ObjectPrx ice_datagram();

    /**
     * Determines whether this proxy uses datagram invocations.
     *
     * @return {@code true} if this proxy uses datagram invocations, {@code false} otherwise
     */
    boolean ice_isDatagram();

    /**
     * Creates a proxy that is identical to this proxy, but uses batch datagram invocations.
     *
     * @return a proxy that uses batch datagram invocations
     */
    ObjectPrx ice_batchDatagram();

    /**
     * Determines whether this proxy uses batch datagram invocations.
     *
     * @return {@code true} if this proxy uses batch datagram invocations, {@code false} otherwise
     */
    boolean ice_isBatchDatagram();

    /**
     * Creates a proxy that is identical to this proxy, except for its compression setting which
     * overrides the compression setting from the proxy endpoints.
     *
     * @param co {@code true} enables compression for the new proxy, {@code false} disables compression.
     * @return a proxy with the specified compression override setting
     */
    ObjectPrx ice_compress(boolean co);

    /**
     * Gets the compression override setting of this proxy.
     *
     * @return The compression override setting. If no optional value is present, no override is set.
     *     Otherwise, {@code true} if compression is enabled, {@code false} otherwise.
     */
    Optional<Boolean> ice_getCompress();

    /**
     * Creates a proxy that is identical to this proxy, except for its connection ID.
     *
     * @param connectionId the connection ID for the new proxy. An empty string removes the connection ID.
     * @return a proxy with the specified connection ID
     */
    ObjectPrx ice_connectionId(String connectionId);

    /**
     * Gets the connection for this proxy. If the proxy does not yet have an established connection,
     * it first attempts to create a connection.
     *
     * <p>You can call this method to establish a connection or associate the proxy with an existing
     * connection and ignore the return value.
     *
     * @return The {@link Connection} for this proxy.
     */
    Connection ice_getConnection();

    /**
     * Returns an executor object that uses the Ice thread pool.
     *
     * @return the {@link Executor} object
     */
    default Executor ice_executor() {
        return _getReference().getThreadPool();
    }

    /**
     * Gets the connection for this proxy asynchronously. If the proxy does not yet have an established connection,
     * it first attempts to create a connection.
     *
     * @return a future that completes with the {@link Connection} for this proxy
     */
    CompletableFuture<Connection> ice_getConnectionAsync();

    /**
     * Gets the cached connection for this proxy. If the proxy does not yet have an established connection,
     * it does not attempt to create a connection.
     *
     * @return the cached {@link Connection} for this proxy, or {@code null} if the proxy does not
     *     have an established connection
     * @see Connection
     */
    Connection ice_getCachedConnection();

    /**
     * Flushes any pending batched requests for this proxy. The call blocks until the flush is complete.
     */
    void ice_flushBatchRequests();

    /**
     * Flushes any pending batched requests for this proxy asynchronously. The call does not block.
     *
     * @return a future that completes when the flush completes
     */
    CompletableFuture<Void> ice_flushBatchRequestsAsync();

    /**
     * Determines whether this proxy equals the passed object. Two proxies are equal if they are equal
     * in all respects, that is, if their object identity, endpoints timeout settings, and so on are all equal.
     *
     * @param r The object to compare this proxy with.
     * @return {@code true} if this proxy is equal to {@code r}, {@code false} otherwise.
     */
    @Override
    boolean equals(java.lang.Object r);

    /** The type ID of the target's interface: "::Ice::Object". */
    static final String ice_staticId = "::Ice::Object";

    /**
     * Gets the Slice type ID associated with this type.
     *
     * @return the Slice type ID
     */
    static String ice_staticId() {
        return ice_staticId;
    }

    /**
     * Creates a new proxy that implements {@link ObjectPrx}.
     *
     * @param communicator the communicator of the new proxy
     * @param proxyString the string representation of the proxy
     * @return the new proxy
     * @throws ParseException if {@code proxyString} is not a valid proxy string.
     */
    public static ObjectPrx createProxy(Communicator communicator, String proxyString) {
        var ref = communicator.getInstance().referenceFactory().create(proxyString, null);
        if (ref == null) {
            throw new ParseException("Invalid empty proxy string.");
        }
        return new _ObjectPrxI(ref);
    }

    /**
     * Creates a new proxy from an existing proxy after confirming the target object's type via a remote invocation.
     *
     * @param obj the source proxy
     * @return a new proxy or {@code null} if the target object does not support this proxy's type
     */
    static ObjectPrx checkedCast(ObjectPrx obj) {
        return checkedCast(obj, noExplicitContext);
    }

    /**
     * Creates a new proxy from an existing proxy after confirming the target object's type via a remote invocation.
     *
     * @param obj the source proxy
     * @param context the request context
     * @return a new proxy or {@code null} if the target object does not support this proxy's type
     */
    static ObjectPrx checkedCast(ObjectPrx obj, Map<String, String> context) {
        return obj != null && obj.ice_isA(ice_staticId, context) ? obj : null;
    }

    /**
     * Creates a new proxy from an existing proxy after confirming the target object's type via a remote invocation.
     *
     * @param obj the source proxy
     * @param facet a facet name
     * @return a new proxy with the specified facet, or {@code null} if the target facet does not support the specified
     *     type
     */
    static ObjectPrx checkedCast(ObjectPrx obj, String facet) {
        return checkedCast(obj, facet, noExplicitContext);
    }

    /**
    * Creates a new proxy from an existing proxy after confirming the target object's type via a remote invocation.
     *
     * @param obj the source proxy
     * @param facet a facet name
     * @param context the request context
     * @return a new proxy with the specified facet, or {@code null} if the target facet does not support the specified
     *     type
     */
    static ObjectPrx checkedCast(ObjectPrx obj, String facet, Map<String, String> context) {
        return obj == null ? null : checkedCast(obj.ice_facet(facet), context);
    }

    /**
     * Creates a new proxy from an existing proxy.
     *
     * @param obj the source proxy
     * @return a new proxy with the desired type or {@code null} if {@code obj} is {@code null}
     */
    static ObjectPrx uncheckedCast(ObjectPrx obj) {
        return obj;
    }

    /**
    * Creates a new proxy from an existing proxy.
     *
     * @param obj the source proxy
     * @param facet a facet name
     * @return a new proxy with the desired type or {@code null} if {@code obj} is {@code null}
     */
    static ObjectPrx uncheckedCast(ObjectPrx obj, String facet) {
        return obj == null ? null : obj.ice_facet(facet);
    }

    /**
     * Writes a proxy to the provided stream.
     *
     * @param ostr the destination stream
     * @param v the proxy to write to the stream
     */
    static void write(OutputStream ostr, ObjectPrx v) {
        ostr.writeProxy(v);
    }

    /**
     * Reads a proxy from the provided stream.
     *
     * @param istr the source stream
     * @return a new proxy from the stream or {@code null} for a null proxy
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
     * Creates a new {@code ObjectPrx} instance.
     * This is a helper method and should not be called directly by user code.
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
     * For example, {@code prx.op(noExplicitContext)} is the same as {@code prx.op()} and
     * does not override the current implicit context (if any).
     */
    static final Map<String, String> noExplicitContext = new HashMap<>();

    /**
     * Ensures that a proxy received over the wire is not null.
     *
     * @param proxy the proxy to check.
     * @param current the current object of the corresponding incoming request.
     * @throws MarshalException if the proxy is null.
     */
    static void checkNotNull(ObjectPrx proxy, Current current) {
        if (proxy == null) {
            throw new MarshalException(
                String.format(
                    "Null proxy passed to '%s' on object '%s'.",
                    current.operation,
                    current.adapter.getCommunicator().identityToString(current.id)));
        }
    }
}
