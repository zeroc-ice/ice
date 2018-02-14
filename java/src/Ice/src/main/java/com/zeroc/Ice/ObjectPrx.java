// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Ice;

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
     * @param id The type ID of the Slice interface to test against.
     * @return <code>true</code> if the target object has the interface
     * specified by <code>id</code> or derives from the interface
     * specified by <code>id</code>.
     **/
    boolean ice_isA(String id);

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param id The type ID of the Slice interface to test against.
     * @param context The context map for the invocation.
     * @return <code>true</code> if the target object has the interface
     * specified by <code>id</code> or derives from the interface
     * specified by <code>id</code>.
     **/
    boolean ice_isA(String id, java.util.Map<String, String> context);

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param id The type ID of the Slice interface to test against.
     * @return A future for the completion of the request.
     **/
    java.util.concurrent.CompletableFuture<Boolean> ice_isAAsync(String id);

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param id The type ID of the Slice interface to test against.
     * @param context The context map for the invocation.
     * @return A future for the completion of the request.
     **/
    java.util.concurrent.CompletableFuture<Boolean> ice_isAAsync(String id, java.util.Map<String, String> context);

    /**
     * Tests whether the target object of this proxy can be reached.
     **/
    void ice_ping();

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param context The context map for the invocation.
     **/
    void ice_ping(java.util.Map<String, String> context);

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @return A future for the completion of the request.
     **/
    java.util.concurrent.CompletableFuture<Void> ice_pingAsync();

    /**
     * Tests whether the target object of this proxy can be reached.
     *
     * @param context The context map for the invocation.
     * @return A future for the completion of the request.
     **/
    java.util.concurrent.CompletableFuture<Void> ice_pingAsync(java.util.Map<String, String> context);

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
     * @param context The context map for the invocation.
     * @return The Slice type IDs of the interfaces supported by the target object, in base-to-derived
     * order. The first element of the returned array is always <code>::Ice::Object</code>.
     **/
    String[] ice_ids(java.util.Map<String, String> context);

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @return A future for the completion of the request.
     **/
    java.util.concurrent.CompletableFuture<String[]> ice_idsAsync();

    /**
     * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
     *
     * @param context The context map for the invocation.
     * @return A future for the completion of the request.
     **/
    java.util.concurrent.CompletableFuture<String[]> ice_idsAsync(java.util.Map<String, String> context);

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @return The Slice type ID of the most-derived interface.
     **/
    String ice_id();

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @param context The context map for the invocation.
     * @return The Slice type ID of the most-derived interface.
     **/
    String ice_id(java.util.Map<String, String> context);

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @return A future for the completion of the request.
     **/
    java.util.concurrent.CompletableFuture<String> ice_idAsync();

    /**
     * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
     *
     * @param context The context map for the invocation.
     * @return A future for the completion of the request.
     **/
    java.util.concurrent.CompletableFuture<String> ice_idAsync(java.util.Map<String, String> context);

    /**
     * Invokes an operation dynamically.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * @return The results of the invocation.
     *
     * @see Blobject
     * @see OperationMode
     **/
    com.zeroc.Ice.Object.Ice_invokeResult ice_invoke(String operation, OperationMode mode, byte[] inParams);

    /**
     * Invokes an operation dynamically.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * @param context The context map for the invocation.
     * @return The results of the invocation.
     *
     * @see Blobject
     * @see OperationMode
     **/
    com.zeroc.Ice.Object.Ice_invokeResult ice_invoke(String operation, OperationMode mode, byte[] inParams,
                                                     java.util.Map<String, String> context);

    /**
     * Invokes an operation dynamically and asynchronously.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * @return A future for the completion of the request.
     *
     * @see Blobject
     * @see OperationMode
     **/
    java.util.concurrent.CompletableFuture<com.zeroc.Ice.Object.Ice_invokeResult> ice_invokeAsync(
        String operation,
        OperationMode mode,
        byte[] inParams);

    /**
     * Invokes an operation dynamically and asynchronously.
     *
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * for the operation. The return value follows any out-parameters.
     * @param context The context map for the invocation.
     * @return A future for the completion of the request.
     *
     * @see Blobject
     * @see OperationMode
     **/
    java.util.concurrent.CompletableFuture<com.zeroc.Ice.Object.Ice_invokeResult> ice_invokeAsync(
        String operation,
        OperationMode mode,
        byte[] inParams,
        java.util.Map<String, String> context);

    /**
     * Returns the identity embedded in this proxy.
     *
     * @return The identity of the target object.
     **/
    Identity ice_getIdentity();

    /**
     * Returns a proxy that is identical to this proxy, except for the identity.
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
     * Returns a proxy that is identical to this proxy, except for the per-proxy context.
     *
     * @param newContext The context for the new proxy.
     * @return The proxy with the new per-proxy context.
     **/
    default ObjectPrx ice_context(java.util.Map<String, String> newContext)
    {
        return _ice_context(newContext);
    }

    /**
     * Returns the facet for this proxy.
     *
     * @return The facet for this proxy. If the proxy uses the default facet, the return value is the empty string.
     **/
    String ice_getFacet();

    /**
     * Returns a proxy that is identical to this proxy, except for the facet.
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
     * Returns a proxy that is identical to this proxy, except for the adapter ID.
     *
     * @param newAdapterId The adapter ID for the new proxy.
     * @return The proxy with the new adapter ID.
     **/
    default ObjectPrx ice_adapterId(String newAdapterId)
    {
        return _ice_adapterId(newAdapterId);
    }

    /**
     * Returns the endpoints used by this proxy.
     *
     * @return The endpoints used by this proxy.
     *
     * @see Endpoint
     **/
    Endpoint[] ice_getEndpoints();

    /**
     * Returns a proxy that is identical to this proxy, except for the endpoints.
     *
     * @param newEndpoints The endpoints for the new proxy.
     * @return The proxy with the new endpoints.
     **/
    default ObjectPrx ice_endpoints(Endpoint[] newEndpoints)
    {
        return _ice_endpoints(newEndpoints);
    }

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
     * Returns a proxy that is identical to this proxy, except for the locator cache timeout.
     *
     * @param newTimeout The new locator cache timeout (in seconds).
     * @return The proxy with the new timeout.
     *
     * @see Locator
     **/
    default ObjectPrx ice_locatorCacheTimeout(int newTimeout)
    {
        return _ice_locatorCacheTimeout(newTimeout);
    }

    /**
     * Returns a proxy that is identical to this proxy, except for the invocation timeout.
     *
     * @param newTimeout The new invocation timeout (in seconds).
     * @return The proxy with the new timeout.
     *
     **/
    default ObjectPrx ice_invocationTimeout(int newTimeout)
    {
        return _ice_invocationTimeout(newTimeout);
    }

    /**
     * Returns whether this proxy caches connections.
     *
     * @return <code>true</code> if this proxy caches connections; <code>false</code> otherwise.
     **/
    boolean ice_isConnectionCached();

    /**
     * Returns a proxy that is identical to this proxy, except for connection caching.
     *
     * @param newCache <code>true</code> if the new proxy should cache connections; <code>false</code> otherwise.
     * @return The proxy with the specified caching policy.
     **/
    default ObjectPrx ice_connectionCached(boolean newCache)
    {
        return _ice_connectionCached(newCache);
    }

    /**
     * Returns how this proxy selects endpoints (randomly or ordered).
     *
     * @return The endpoint selection policy.
     *
     * @see EndpointSelectionType
     **/
    EndpointSelectionType ice_getEndpointSelection();

    /**
     * Returns a proxy that is identical to this proxy, except for the endpoint selection policy.
     *
     * @param newType The new endpoint selection policy.
     * @return The proxy with the specified endpoint selection policy.
     *
     * @see EndpointSelectionType
     **/
    default ObjectPrx ice_endpointSelection(EndpointSelectionType newType)
    {
        return _ice_endpointSelection(newType);
    }

    /**
     * Returns whether this proxy uses only secure endpoints.
     *
     * @return <code>True</code> if this proxy communicates only via secure endpoints; <code>false</code> otherwise.
     **/
    boolean ice_isSecure();

    /**
     * Returns a proxy that is identical to this proxy, except for how it selects endpoints.
     *
     * @param b If <code>b</code> is <code>true</code>, only endpoints that use a secure transport are
     * used by the new proxy. If <code>b</code> is false, the returned proxy uses both secure and insecure
     * endpoints.
     * @return The proxy with the specified selection policy.
     **/
    default ObjectPrx ice_secure(boolean b)
    {
        return _ice_secure(b);
    }

    /**
     * Returns a proxy that is identical to this proxy, except for the encoding used to marshal
     * parameters.
     *
     * @param e The encoding version to use to marshal request parameters.
     * @return The proxy with the specified encoding version.
     **/
    default ObjectPrx ice_encodingVersion(EncodingVersion e)
    {
        return _ice_encodingVersion(e);
    }

    /**
     * Returns the encoding version used to marshal requests parameters.
     *
     * @return The encoding version.
     **/
    EncodingVersion ice_getEncodingVersion();

    /**
     * Returns whether this proxy prefers secure endpoints.
     *
     * @return <code>true</code> if the proxy always attempts to invoke via secure endpoints before it
     * attempts to use insecure endpoints; <code>false</code> otherwise.
     **/
    boolean ice_isPreferSecure();

    /**
     * Returns a proxy that is identical to this proxy, except for its endpoint selection policy.
     *
     * @param b If <code>b</code> is <code>true</code>, the new proxy will use secure endpoints for invocations
     * and only use insecure endpoints if an invocation cannot be made via secure endpoints. If <code>b</code> is
     * <code>false</code>, the proxy prefers insecure endpoints to secure ones.
     * @return The proxy with the specified selection policy.
     **/
    default ObjectPrx ice_preferSecure(boolean b)
    {
        return _ice_preferSecure(b);
    }

    /**
     * Returns the router for this proxy.
     *
     * @return The router for the proxy. If no router is configured for the proxy, the return value
     * is <code>null</code>.
     **/
    RouterPrx ice_getRouter();

    /**
     * Returns a proxy that is identical to this proxy, except for the router.
     *
     * @param router The router for the new proxy.
     * @return The proxy with the specified router.
     **/
    default ObjectPrx ice_router(RouterPrx router)
    {
        return _ice_router(router);
    }

    /**
     * Returns the locator for this proxy.
     *
     * @return The locator for this proxy. If no locator is configured, the return value is <code>null</code>.
     **/
    LocatorPrx ice_getLocator();

    /**
     * Returns a proxy that is identical to this proxy, except for the locator.
     *
     * @param locator The locator for the new proxy.
     * @return The proxy with the specified locator.
     **/
    default ObjectPrx ice_locator(LocatorPrx locator)
    {
        return _ice_locator(locator);
    }

    /**
     * Returns whether this proxy uses collocation optimization.
     *
     * @return <code>true</code> if the proxy uses collocation optimization; <code>false</code> otherwise.
     **/
    boolean ice_isCollocationOptimized();

    /**
     * Returns a proxy that is identical to this proxy, except for collocation optimization.
     *
     * @param b <code>true</code> if the new proxy enables collocation optimization; <code>false</code> otherwise.
     * @return The proxy the specified collocation optimization.
     **/
    default ObjectPrx ice_collocationOptimized(boolean b)
    {
        return _ice_collocationOptimized(b);
    }

    /**
     * Returns a proxy that is identical to this proxy, but uses twoway invocations.
     *
     * @return A proxy that uses twoway invocations.
     **/
    default ObjectPrx ice_twoway()
    {
        return _ice_twoway();
    }

    /**
     * Returns whether this proxy uses twoway invocations.
     * @return <code>true</code> if this proxy uses twoway invocations; <code>false</code> otherwise.
     **/
    boolean ice_isTwoway();

    /**
     * Returns a proxy that is identical to this proxy, but uses oneway invocations.
     *
     * @return A proxy that uses oneway invocations.
     **/
    default ObjectPrx ice_oneway()
    {
        return _ice_oneway();
    }

    /**
     * Returns whether this proxy uses oneway invocations.
     * @return <code>true</code> if this proxy uses oneway invocations; <code>false</code> otherwise.
     **/
    boolean ice_isOneway();

    /**
     * Returns a proxy that is identical to this proxy, but uses batch oneway invocations.
     *
     * @return A new proxy that uses batch oneway invocations.
     **/
    default ObjectPrx ice_batchOneway()
    {
        return _ice_batchOneway();
    }

    /**
     * Returns whether this proxy uses batch oneway invocations.
     * @return <code>true</code> if this proxy uses batch oneway invocations; <code>false</code> otherwise.
     **/
    boolean ice_isBatchOneway();

    /**
     * Returns a proxy that is identical to this proxy, but uses datagram invocations.
     *
     * @return A new proxy that uses datagram invocations.
     **/
    default ObjectPrx ice_datagram()
    {
        return _ice_datagram();
    }

    /**
     * Returns whether this proxy uses datagram invocations.
     * @return <code>true</code> if this proxy uses datagram invocations; <code>false</code> otherwise.
     **/
    boolean ice_isDatagram();

    /**
     * Returns a proxy that is identical to this proxy, but uses batch datagram invocations.
     *
     * @return A new proxy that uses batch datagram invocations.
     **/
    default ObjectPrx ice_batchDatagram()
    {
        return _ice_batchDatagram();
    }

    /**
     * Returns whether this proxy uses batch datagram invocations.
     * @return <code>true</code> if this proxy uses batch datagram invocations; <code>false</code> otherwise.
     **/
    boolean ice_isBatchDatagram();

    /**
     * Returns a proxy that is identical to this proxy, except for compression.
     *
     * @param co <code>true</code> enables compression for the new proxy; <code>false</code> disables compression.
     * @return A proxy with the specified compression setting.
     **/
    default ObjectPrx ice_compress(boolean co)
    {
        return _ice_compress(co);
    }

    /**
     * Returns a proxy that is identical to this proxy, except for its connection timeout setting.
     *
     * @param t The connection timeout for the proxy in milliseconds.
     * @return A proxy with the specified timeout.
     **/
    default ObjectPrx ice_timeout(int t)
    {
        return _ice_timeout(t);
    }

    /**
     * Returns a proxy that is identical to this proxy, except for its connection ID.
     *
     * @param connectionId The connection ID for the new proxy. An empty string removes the
     * connection ID.
     *
     * @return A proxy with the specified connection ID.
     **/
    default ObjectPrx ice_connectionId(String connectionId)
    {
        return _ice_connectionId(connectionId);
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
    Connection ice_getConnection();

    /**
     * Returns an executor object that uses the Ice thread pool.
     *
     * @return The Executor object.
     **/
    default java.util.concurrent.Executor ice_executor()
    {
        return _getReference().getThreadPool();
    }

    /**
     * Asynchronously gets the connection for this proxy. The call does not block.
     *
     * @return A future for the completion of the request.
     **/
    java.util.concurrent.CompletableFuture<Connection> ice_getConnectionAsync();

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
     * @return A future for the completion of the request.
     **/
    java.util.concurrent.CompletableFuture<Void> ice_flushBatchRequestsAsync();

    /**
     * Returns whether this proxy equals the passed object. Two proxies are equal if they are equal in all respects,
     * that is, if their object identity, endpoints timeout settings, and so on are all equal.
     *
     * @param r The object to compare this proxy with.
     * @return <code>true</code> if this proxy is equal to <code>r</code>; <code>false</code> otherwise.
     **/
    @Override
    boolean equals(java.lang.Object r);

    static final String ice_staticId = "::Ice::Object";

    /**
     * Returns the Slice type ID associated with this type.
     * @return The Slice type ID.
     **/
    static String ice_staticId()
    {
        return ice_staticId;
    }

    /**
     * Casts a proxy to {@link ObjectPrx}. For user-defined types, this call contacts
     * the server and will throw an Ice run-time exception if the target
     * object does not exist or the server cannot be reached.
     *
     * @param obj The proxy to cast to @{link ObjectPrx}.
     * @return <code>obj</code>.
     **/
    static ObjectPrx checkedCast(ObjectPrx obj)
    {
        return obj;
    }

    /**
     * Casts a proxy to {@link ObjectPrx}. For user-defined types, this call contacts
     * the server and throws an Ice run-time exception if the target
     * object does not exist or the server cannot be reached.
     *
     * @param obj The proxy to cast to {@link ObjectPrx}.
     * @param context The <code>Context</code> map for the invocation.
     * @return <code>obj</code>.
     **/
    static ObjectPrx checkedCast(ObjectPrx obj, java.util.Map<String, String> context)
    {
        return obj;
    }

    /**
     * Creates a new proxy that is identical to the passed proxy, except
     * for its facet. This call contacts
     * the server and throws an Ice run-time exception if the target
     * object does not exist, the specified facet does not exist, or the server cannot be reached.
     *
     * @param obj The proxy to cast to {@link ObjectPrx}.
     * @param facet The facet for the new proxy.
     * @return The new proxy with the specified facet.
     **/
    static ObjectPrx checkedCast(ObjectPrx obj, String facet)
    {
        return checkedCast(obj, facet, noExplicitContext);
    }

    /**
     * Creates a new proxy that is identical to the passed proxy, except
     * for its facet. This call contacts
     * the server and throws an Ice run-time exception if the target
     * object does not exist, the specified facet does not exist, or the server cannot be reached.
     *
     * @param obj The proxy to cast to {@link ObjectPrx}.
     * @param facet The facet for the new proxy.
     * @param context The <code>Context</code> map for the invocation.
     * @return The new proxy with the specified facet.
     **/
    static ObjectPrx checkedCast(ObjectPrx obj, String facet, java.util.Map<String, String> context)
    {
        ObjectPrx r = null;
        if(obj != null)
        {
            ObjectPrx p = obj.ice_facet(facet);
            try
            {
                boolean ok = p.ice_isA(ice_staticId, context);
                assert(ok);
                r = new _ObjectPrxI();
                r._copyFrom(p);
            }
            catch(FacetNotExistException ex)
            {
            }
        }
        return r;
    }

    /**
     * Casts a proxy to {@link ObjectPrx}. This call does
     * not contact the server and always succeeds.
     *
     * @param obj The proxy to cast to {@link ObjectPrx}.
     * @return <code>obj</code>.
     **/
    static ObjectPrx uncheckedCast(ObjectPrx obj)
    {
        return obj;
    }

    /**
     * Creates a new proxy that is identical to the passed proxy, except
     * for its facet. This call does not contact the server and always succeeds.
     *
     * @param obj The proxy to cast to {@link ObjectPrx}.
     * @param facet The facet for the new proxy.
     * @return The new proxy with the specified facet.
     **/
    static ObjectPrx uncheckedCast(ObjectPrx obj, String facet)
    {
        ObjectPrx r = null;
        if(obj != null)
        {
            ObjectPrx p = obj.ice_facet(facet);
            r = new _ObjectPrxI();
            r._copyFrom(p);
        }
        return r;
    }

    /**
     * Writes a proxy to the stream.
     *
     * @param ostr The destination stream.
     * @param v The proxy to write to the stream.
     **/
    static void write(OutputStream ostr, ObjectPrx v)
    {
        ostr.writeProxy(v);
    }

    /**
     * Reads a proxy from the stream.
     *
     * @param istr The source stream.
     * @return A new proxy or null for a nil proxy.
     **/
    static ObjectPrx read(InputStream istr)
    {
        return istr.readProxy();
    }

    static <T> T _checkedCast(ObjectPrx obj, String id, Class<T> proxy, Class<?> impl)
    {
        return _checkedCast(obj, false, null, noExplicitContext, id, proxy, impl);
    }

    static <T> T _checkedCast(ObjectPrx obj, java.util.Map<String, String> ctx, String id, Class<T> proxy,
                               Class<?> impl)
    {
        return _checkedCast(obj, false, null, ctx, id, proxy, impl);
    }

    static <T> T _checkedCast(ObjectPrx obj, String facet, String id, Class<T> proxy, Class<?> impl)
    {
        return _checkedCast(obj, true, facet, noExplicitContext, id, proxy, impl);
    }

    static <T> T _checkedCast(ObjectPrx obj, String facet, java.util.Map<String, String> ctx, String id,
                               Class<T> proxy, Class<?> impl)
    {
        return _checkedCast(obj, true, facet, ctx, id, proxy, impl);
    }

    static <T> T _checkedCast(ObjectPrx obj, boolean explicitFacet, String facet, java.util.Map<String, String> ctx,
                               String id, Class<T> proxy, Class<?> impl)
    {
        T r = null;
        if(obj != null)
        {
            if(explicitFacet)
            {
                obj = obj.ice_facet(facet);
            }
            if(proxy.isInstance(obj))
            {
                r = proxy.cast(obj);
            }
            else
            {
                try
                {
                    boolean ok = obj.ice_isA(id, ctx);
                    if(ok)
                    {
                        ObjectPrx h = null;
                        try
                        {
                            h = _ObjectPrxI.class.cast(impl.getDeclaredConstructor().newInstance());
                        }
                        catch(NoSuchMethodException ex)
                        {
                            throw new SyscallException(ex);
                        }
                        catch(java.lang.reflect.InvocationTargetException ex)
                        {
                            throw new SyscallException(ex);
                        }
                        catch(InstantiationException ex)
                        {
                            throw new SyscallException(ex);
                        }
                        catch(IllegalAccessException ex)
                        {
                            throw new SyscallException(ex);
                        }
                        h._copyFrom(obj);
                        r = proxy.cast(h);
                    }
                }
                catch(FacetNotExistException ex)
                {
                }
            }
        }
        return r;
    }

    static <T> T _uncheckedCast(ObjectPrx obj, Class<T> proxy, Class<?> impl)
    {
        return _uncheckedCast(obj, false, null, proxy, impl);
    }

    static <T> T _uncheckedCast(ObjectPrx obj, String facet, Class<T> proxy, Class<?> impl)
    {
        return _uncheckedCast(obj, true, facet, proxy, impl);
    }

    static <T> T _uncheckedCast(ObjectPrx obj, boolean explicitFacet, String facet, Class<T> proxy, Class<?> impl)
    {
        T r = null;
        if(obj != null)
        {
            try
            {
                if(explicitFacet)
                {
                    ObjectPrx h = _ObjectPrxI.class.cast(impl.getDeclaredConstructor().newInstance());
                    h._copyFrom(obj.ice_facet(facet));
                    r = proxy.cast(h);
                }
                else
                {
                    if(proxy.isInstance(obj))
                    {
                        r = proxy.cast(obj);
                    }
                    else
                    {
                        ObjectPrx h = _ObjectPrxI.class.cast(impl.getDeclaredConstructor().newInstance());
                        h._copyFrom(obj);
                        r = proxy.cast(h);
                    }
                }
            }
            catch(NoSuchMethodException ex)
            {
                throw new SyscallException(ex);
            }
            catch(java.lang.reflect.InvocationTargetException ex)
            {
                throw new SyscallException(ex);
            }
            catch(InstantiationException ex)
            {
                throw new SyscallException(ex);
            }
            catch(IllegalAccessException ex)
            {
                throw new SyscallException(ex);
            }
        }
        return r;
    }

    void _write(OutputStream os);
    void _copyFrom(ObjectPrx p);
    com.zeroc.IceInternal.Reference _getReference();
    ObjectPrx _newInstance(com.zeroc.IceInternal.Reference r);

    default ObjectPrx _ice_context(java.util.Map<String, String> newContext)
    {
        return _newInstance(_getReference().changeContext(newContext));
    }

    default ObjectPrx _ice_adapterId(String newAdapterId)
    {
        if(newAdapterId == null)
        {
            newAdapterId = "";
        }

        if(newAdapterId.equals(_getReference().getAdapterId()))
        {
            return this;
        }
        else
        {
            return _newInstance(_getReference().changeAdapterId(newAdapterId));
        }
    }

    default ObjectPrx _ice_endpoints(Endpoint[] newEndpoints)
    {
        if(java.util.Arrays.equals(newEndpoints, _getReference().getEndpoints()))
        {
            return this;
        }
        else
        {
            com.zeroc.IceInternal.EndpointI[] edpts = new com.zeroc.IceInternal.EndpointI[newEndpoints.length];
            edpts = java.util.Arrays.asList(newEndpoints).toArray(edpts);
            return _newInstance(_getReference().changeEndpoints(edpts));
        }
    }

    default ObjectPrx _ice_locatorCacheTimeout(int newTimeout)
    {
        if(newTimeout < -1)
        {
            throw new IllegalArgumentException("invalid value passed to ice_locatorCacheTimeout: " + newTimeout);
        }
        if(newTimeout == _getReference().getLocatorCacheTimeout())
        {
            return this;
        }
        else
        {
            return _newInstance(_getReference().changeLocatorCacheTimeout(newTimeout));
        }
    }

    default ObjectPrx _ice_invocationTimeout(int newTimeout)
    {
        if(newTimeout < 1 && newTimeout != -1 && newTimeout != -2)
        {
            throw new IllegalArgumentException("invalid value passed to ice_invocationTimeout: " + newTimeout);
        }
        if(newTimeout == _getReference().getInvocationTimeout())
        {
            return this;
        }
        else
        {
            return _newInstance(_getReference().changeInvocationTimeout(newTimeout));
        }
    }

    default ObjectPrx _ice_connectionCached(boolean newCache)
    {
        if(newCache == _getReference().getCacheConnection())
        {
            return this;
        }
        else
        {
            return _newInstance(_getReference().changeCacheConnection(newCache));
        }
    }

    default ObjectPrx _ice_endpointSelection(EndpointSelectionType newType)
    {
        if(newType == _getReference().getEndpointSelection())
        {
            return this;
        }
        else
        {
            return _newInstance(_getReference().changeEndpointSelection(newType));
        }
    }

    default ObjectPrx _ice_secure(boolean b)
    {
        if(b == _getReference().getSecure())
        {
            return this;
        }
        else
        {
            return _newInstance(_getReference().changeSecure(b));
        }
    }

    default ObjectPrx _ice_encodingVersion(EncodingVersion e)
    {
        if(e.equals(_getReference().getEncoding()))
        {
            return this;
        }
        else
        {
            return _newInstance(_getReference().changeEncoding(e));
        }
    }

    default ObjectPrx _ice_preferSecure(boolean b)
    {
        if(b == _getReference().getPreferSecure())
        {
            return this;
        }
        else
        {
            return _newInstance(_getReference().changePreferSecure(b));
        }
    }

    default ObjectPrx _ice_router(RouterPrx router)
    {
        com.zeroc.IceInternal.Reference ref = _getReference().changeRouter(router);
        if(ref.equals(_getReference()))
        {
            return this;
        }
        else
        {
            return _newInstance(ref);
        }
    }

    default ObjectPrx _ice_locator(LocatorPrx locator)
    {
        com.zeroc.IceInternal.Reference ref = _getReference().changeLocator(locator);
        if(ref.equals(_getReference()))
        {
            return this;
        }
        else
        {
            return _newInstance(ref);
        }
    }

    default ObjectPrx _ice_collocationOptimized(boolean b)
    {
        if(b == _getReference().getCollocationOptimized())
        {
            return this;
        }
        else
        {
            return _newInstance(_getReference().changeCollocationOptimized(b));
        }
    }

    default ObjectPrx _ice_twoway()
    {
        if(_getReference().getMode() == com.zeroc.IceInternal.Reference.ModeTwoway)
        {
            return this;
        }
        else
        {
            return _newInstance(_getReference().changeMode(com.zeroc.IceInternal.Reference.ModeTwoway));
        }
    }

    default ObjectPrx _ice_oneway()
    {
        if(_getReference().getMode() == com.zeroc.IceInternal.Reference.ModeOneway)
        {
            return this;
        }
        else
        {
            return _newInstance(_getReference().changeMode(com.zeroc.IceInternal.Reference.ModeOneway));
        }
    }

    default ObjectPrx _ice_batchOneway()
    {
        if(_getReference().getMode() == com.zeroc.IceInternal.Reference.ModeBatchOneway)
        {
            return this;
        }
        else
        {
            return _newInstance(_getReference().changeMode(com.zeroc.IceInternal.Reference.ModeBatchOneway));
        }
    }

    default ObjectPrx _ice_datagram()
    {
        if(_getReference().getMode() == com.zeroc.IceInternal.Reference.ModeDatagram)
        {
            return this;
        }
        else
        {
            return _newInstance(_getReference().changeMode(com.zeroc.IceInternal.Reference.ModeDatagram));
        }
    }

    default ObjectPrx _ice_batchDatagram()
    {
        if(_getReference().getMode() == com.zeroc.IceInternal.Reference.ModeBatchDatagram)
        {
            return this;
        }
        else
        {
            return _newInstance(_getReference().changeMode(com.zeroc.IceInternal.Reference.ModeBatchDatagram));
        }
    }

    default ObjectPrx _ice_compress(boolean co)
    {
        com.zeroc.IceInternal.Reference ref = _getReference().changeCompress(co);
        if(ref.equals(_getReference()))
        {
            return this;
        }
        else
        {
            return _newInstance(ref);
        }
    }

    default ObjectPrx _ice_timeout(int t)
    {
        if(t < 1 && t != -1)
        {
            throw new IllegalArgumentException("invalid value passed to ice_timeout: " + t);
        }
        com.zeroc.IceInternal.Reference ref = _getReference().changeTimeout(t);
        if(ref.equals(_getReference()))
        {
            return this;
        }
        else
        {
            return _newInstance(ref);
        }
    }

    default ObjectPrx _ice_connectionId(String connectionId)
    {
        com.zeroc.IceInternal.Reference ref = _getReference().changeConnectionId(connectionId);
        if(ref.equals(_getReference()))
        {
            return this;
        }
        else
        {
            return _newInstance(ref);
        }
    }

    static final java.util.Map<String, String> noExplicitContext = new java.util.HashMap<>();
}
