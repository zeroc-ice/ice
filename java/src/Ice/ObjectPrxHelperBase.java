// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

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
     * @deprecated
     **/
    public final int
    ice_getHash()
    {
        return _reference.hashCode();
    }

    /**
     * Returns the communicator that created this proxy.
     *
     * @return The communicator that created this proxy.
     **/
    public final Communicator ice_getCommunicator()
    {
        return _reference.getCommunicator();
    }

    /**
     * Returns the stringified form of this proxy.
     *
     * @return The stringified proxy.
     **/
    public final String toString()
    {
        return _reference.toString();
    }

    /**
     * Returns the stringified form of this proxy.
     *
     * @return The stringified proxy.
     **/
    public final String ice_toString()
    {
        return toString();
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

    private boolean
    ice_isA(String __id, java.util.Map<String, String> __context, boolean __explicitCtx)
    {
        if(__explicitCtx && __context == null)
        {
            __context = _emptyContext;
        }
        
        int __cnt = 0;
        while(true)
        {
            _ObjectDel __del = null;
            try
            {
                __checkTwowayOnly("ice_isA");
                __del = __getDelegate(false);
                return __del.ice_isA(__id, __context);
            }
            catch(IceInternal.LocalExceptionWrapper __ex)
            {
                __cnt = __handleExceptionWrapperRelaxed(__del, __ex, null, __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__del, __ex, null, __cnt);
            }
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

    private void
    ice_ping(java.util.Map<String, String> __context, boolean __explicitCtx)
    {
        if(__explicitCtx && __context == null)
        {
            __context = _emptyContext;
        }

        int __cnt = 0;
        while(true)
        {
            _ObjectDel __del = null;
            try
            {
                __del = __getDelegate(false);
                __del.ice_ping(__context);
                return;
            }
            catch(IceInternal.LocalExceptionWrapper __ex)
            {
                __cnt = __handleExceptionWrapperRelaxed(__del, __ex, null, __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__del, __ex, null, __cnt);
            }
        }
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

    private String[]
    ice_ids(java.util.Map<String, String> __context, boolean __explicitCtx)
    {
        if(__explicitCtx && __context == null)
        {
            __context = _emptyContext;
        }

        int __cnt = 0;
        while(true)
        {
            _ObjectDel __del = null;
            try
            {
                __checkTwowayOnly("ice_ids");
                __del = __getDelegate(false);
                return __del.ice_ids(__context);
            }
            catch(IceInternal.LocalExceptionWrapper __ex)
            {
                __cnt = __handleExceptionWrapperRelaxed(__del, __ex, null, __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__del, __ex, null, __cnt);
            }
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

    private String
    ice_id(java.util.Map<String, String> __context, boolean __explicitCtx)
    {
        if(__explicitCtx && __context == null)
        {
            __context = _emptyContext;
        }

        int __cnt = 0;
        while(true)
        {
            _ObjectDel __del = null;
            try
            {
                __checkTwowayOnly("ice_id");
                __del = __getDelegate(false);
                return __del.ice_id(__context);
            }
            catch(IceInternal.LocalExceptionWrapper __ex)
            {
                __cnt = __handleExceptionWrapperRelaxed(__del, __ex, null, __cnt);
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__del, __ex, null, __cnt);
            }
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
     * was no need to queue the request, the return value is <code>true</code>;
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

        int __cnt = 0;
        while(true)
        {
            _ObjectDel __del = null;
            try
            {
                __del = __getDelegate(false);
                return __del.ice_invoke(operation, mode, inParams, outParams, context);
            }
            catch(IceInternal.LocalExceptionWrapper __ex)
            {
                if(mode == OperationMode.Nonmutating || mode == OperationMode.Idempotent)
                {
                    __cnt = __handleExceptionWrapperRelaxed(__del, __ex, null, __cnt);
                }
                else
                {
                    __handleExceptionWrapper(__del, __ex, null);
                }
            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__del, __ex, null, __cnt);
            }
        }
    }

    /**
     * Invokes an operation dynamically and asynchronously.
     *
     * @param cb The callback object to notify when the operation completes.
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * @return If the operation completed successfully, the return value
     * is <code>true</code>. If the operation raises a user exception,
     * the return value is <code>false</code>; in this case, <code>outParams</code>
     * contains the encoded user exception. If the operation raised an
     * it throws it directly.
     *
     * @see AMI_Object_ice_invoke
     * @see OperationMode
     **/
    public final boolean
    ice_invoke_async(AMI_Object_ice_invoke cb, String operation, OperationMode mode, byte[] inParams)
    {
        __checkTwowayOnly("ice_invoke_async");
        return cb.__invoke(this, operation, mode, inParams, null);
    }

    /**
     * Invokes an operation dynamically and asynchronously.
     *
     * @param cb The callback object to notify when the operation completes.
     * @param operation The name of the operation to invoke.
     * @param mode The operation mode (normal or idempotent).
     * @param inParams The encoded in-parameters for the operation.
     * @param __context The context map for the invocation.
     * @return If the operation completed successfully, the return value
     * is <code>true</code>. If the operation raises a user exception,
     * the return value is <code>false</code>; in this case, <code>outParams</code>
     * contains the encoded user exception. If the operation raised an
     * it throws it directly.
     *
     * @see AMI_Object_ice_invoke
     * @see OperationMode
     **/
    public final boolean
    ice_invoke_async(AMI_Object_ice_invoke cb, String operation, OperationMode mode, byte[] inParams,
                     java.util.Map<String, String> context)
    {
        if(context == null)
        {
            context = _emptyContext;
        }
        __checkTwowayOnly("ice_invoke_async");
        return cb.__invoke(this, operation, mode, inParams, context);
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
        int __cnt = 0;
        while(true)
        {
            _ObjectDel __del = null;
            try
            {
                __del = __getDelegate(false);
                return __del.__getRequestHandler().getConnection(true); // Wait for the connection to be established.

            }
            catch(LocalException __ex)
            {
                __cnt = __handleException(__del, __ex, null, __cnt);
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
        _ObjectDel __del = null;
        int __cnt = -1; // Don't retry.
        try
        {
            __del = __getDelegate(false);
            __del.ice_flushBatchRequests();
            return;
        }
        catch(LocalException __ex)
        {
            __cnt = __handleException(__del, __ex, null, __cnt);
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
        return cb.__invoke(this);
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
    __handleException(_ObjectDel delegate, LocalException ex, IceInternal.OutgoingAsync out, int cnt)
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

        if(cnt == -1) // Don't retry if the retry count is -1.
        {
            throw ex;
        }

        try
        {
            return _reference.getInstance().proxyFactory().checkRetryAfterException(ex, _reference, out, cnt);
        }
        catch(CommunicatorDestroyedException e)
        {
            //
            // The communicator is already destroyed, so we cannot
            // retry.
            //
            throw ex;
        }

    }

    public final void
    __handleExceptionWrapper(_ObjectDel delegate, IceInternal.LocalExceptionWrapper ex, IceInternal.OutgoingAsync out)
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
            throw ex.get();
        }

        if(out != null)
        {
            out.__send();
        }
    }

    public final int
    __handleExceptionWrapperRelaxed(_ObjectDel delegate, IceInternal.LocalExceptionWrapper ex, 
                                    IceInternal.OutgoingAsync out, int cnt)
    {
        if(!ex.retry())
        {
            return __handleException(delegate, ex.get(), out, cnt);
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

            if(out != null)
            {
                out.__send();
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
            throw new java.io.IOException("Cannot deserialize proxy: Ice.ObjectInputStream not found");
        }
        catch(LocalException ex)
        {
            java.io.IOException e = new java.io.IOException("Failure occurred while deserializing proxy");
            e.initCause(ex);
            throw e;
        }
    }

    protected static final java.util.Map<String, String> _emptyContext = new java.util.HashMap<String, String>();

    private IceInternal.Reference _reference;
    private _ObjectDel _delegate;
}
