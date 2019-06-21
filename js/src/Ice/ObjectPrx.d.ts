//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice"
{
    namespace Ice
    {
        class ObjectPrx
        {
            static ice_staticId():string;

            /**
             * Tests whether this object supports a specific Slice interface.
             * @param typeId The type ID of the Slice interface to test against.
             * @param context The context map for the invocation.
             * @return The asynchronous result object for the invocation.
             */
            ice_isA(id:string, context?:Map<string, string>):AsyncResult<boolean>;

            /**
             * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
             * @param context The context map for the invocation.
             * @return The asynchronous result object for the invocation.
             */
            ice_id(context?:Map<string, string>):AsyncResult<string>;

            /**
             * Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
             * @param context The context map for the invocation.
             * @return The asynchronous result object for the invocation.
             */
            ice_ids(context?:Map<string, string>):AsyncResult<string[]>;

            /**
             * Tests whether the target object of this proxy can be reached.
             * @param context The context map for the invocation.
             * @return The asynchronous result object for the invocation.
             */
            ice_ping(context?:Map<string, string>):AsyncResult<void>;

            /**
             * Obtains the communicator that created this proxy.
             * @return The communicator that created this proxy.
             */
            ice_getCommunicator():Communicator;

            /**
             * Obtains a stringified version of this proxy.
             * @return A stringified proxy.
             */
            ice_toString():string;

            /**
             * Obtains a proxy that is identical to this proxy, except for the identity.
             * @param id The identity for the new proxy.
             * @return A proxy with the new identity.
             */
            ice_identity(id:Identity):this;

            /**
             * Obtains the identity embedded in this proxy.
             * @return The identity of the target object.
             */
            ice_getIdentity():Identity;

            /**
             * Obtains a proxy that is identical to this proxy, except for the adapter ID.
             * @param id The adapter ID for the new proxy.
             * @return A proxy with the new adapter ID.
             */
            ice_adapterId(id:string):this;

            /**
             * Obtains the adapter ID for this proxy.
             * @return The adapter ID. If the proxy does not have an adapter ID, the return value is the empty string.
             */
            ice_getAdapterId():string;

            /**
             * Obtains a proxy that is identical to this proxy, except for the endpoints.
             * @param endpoints The endpoints for the new proxy.
             * @return A proxy with the new endpoints.
             */
            ice_endpoints(endpoints:Endpoint[]):this;

            /**
             * Obtains the endpoints used by this proxy.
             * @return The endpoints used by this proxy.
             */
            ice_getEndpoints():Endpoint[];

            /**
             * Obtains a proxy that is identical to this proxy, except for the endpoint selection policy.
             * @param type The new endpoint selection policy.
             * @return A proxy with the specified endpoint selection policy.
             */
            ice_endpointSelection(type:EndpointSelectionType):this;

            /**
             * Obtains the endpoint selection policy for this proxy (randomly or ordered).
             * @return The endpoint selection policy.
             */
            ice_getEndpointSelection():EndpointSelectionType;

            /**
             * Obtains a proxy that is identical to this proxy, except for the per-proxy context.
             * @param context The context for the new proxy.
             * @return A proxy with the new per-proxy context.
             */
            ice_context(ctx:Map<string, string>):this;

            /**
             * Obtains the per-proxy context for this proxy.
             * @return The per-proxy context.
             */
            ice_getContext():Map<string, string>;

            /**
             * Obtains a proxy that is identical to this proxy, except for the facet.
             * @param facet The facet for the new proxy.
             * @return A proxy with the new facet.
             */
            ice_facet(facet:string):this;

            /**
             * Obtains the facet for this proxy.
             * @return The facet for this proxy. If the proxy uses the default facet, the return value is the empty string.
             */
            ice_getFacet():string;

            /**
             * Obtains a proxy that is identical to this proxy, but uses twoway invocations.
             * @return A proxy that uses twoway invocations.
             */
            ice_twoway():this;

            /**
             * Determines whether this proxy uses twoway invocations.
             * @return True if this proxy uses twoway invocations, false otherwise.
             */
            ice_isTwoway():boolean;

            /**
             * Obtains a proxy that is identical to this proxy, but uses oneway invocations.
             * @return A proxy that uses oneway invocations.
             */
            ice_oneway():this;

            /**
             * Determines whether this proxy uses oneway invocations.
             * @return True if this proxy uses oneway invocations, false otherwise.
             */
            ice_isOneway():boolean;

            /**
             * Obtains a proxy that is identical to this proxy, but uses batch oneway invocations.
             * @return A proxy that uses batch oneway invocations.
             */
            ice_batchOneway():this;

            /**
             * Determines whether this proxy uses batch oneway invocations.
             * @return True if this proxy uses batch oneway invocations, false otherwise.
             */
            ice_isBatchOneway():boolean;

            /**
             * Obtains a proxy that is identical to this proxy, but uses datagram invocations.
             * @return A proxy that uses datagram invocations.
             */
            ice_datagram():this;

            /**
             * Determines whether this proxy uses datagram invocations.
             * @return True if this proxy uses datagram invocations, false otherwise.
             */
            ice_isDatagram():boolean;

            /**
             * Obtains a proxy that is identical to this proxy, but uses batch datagram invocations.
             * @return A proxy that uses batch datagram invocations.
             */
            ice_batchDatagram():this;

            /**
             * Determines whether this proxy uses batch datagram invocations.
             * @return True if this proxy uses batch datagram invocations, false otherwise.
             */
            ice_isBatchDatagram():boolean;

            /**
             * Obtains a proxy that is identical to this proxy, except for how it selects endpoints.
             * @param secure If true, only endpoints that use a secure transport are used by the new proxy.
             * If false, the returned proxy uses both secure and insecure endpoints.
             * @return A proxy with the specified security policy.
             */
            ice_secure(secure:boolean):this;

            /**
             * Obtains the encoding version used to marshal request parameters.
             * @return The encoding version.
             */
            ice_getEncodingVersion():EncodingVersion;

            /**
             * Obtains a proxy that is identical to this proxy, except for the encoding used to marshal
             * parameters.
             * @param version The encoding version to use to marshal request parameters.
             * @return A proxy with the specified encoding version.
             */
            ice_encodingVersion(encoding:EncodingVersion):this;

            /**
             * Determines whether this proxy uses only secure endpoints.
             * @return True if this proxy communicates only via secure endpoints, false otherwise.
             */
            ice_isSecure():boolean;

            /**
             * Obtains a proxy that is identical to this proxy, except for its endpoint selection policy.
             * @param secure If true, the new proxy will use secure endpoints for invocations and only use
             * insecure endpoints if an invocation cannot be made via secure endpoints. If false, the
             * proxy prefers insecure endpoints to secure ones.
             * @return A proxy with the specified selection policy.
             */
            ice_preferSecure(secure:boolean):this;

            /**
             * Determines whether this proxy prefers secure endpoints.
             * @return True if the proxy always attempts to invoke via secure endpoints before it
             * attempts to use insecure endpoints, false otherwise.
             */
            ice_isPreferSecure():boolean;

            /**
             * Obtains a proxy that is identical to this proxy, except for its compression setting which
             * overrides the compression setting from the proxy endpoints.
             * @param b True enables compression for the new proxy, false disables compression.
             * @return A proxy with the specified compression override setting.
             */
            ice_compress(compress:boolean):this;

            /**
             * Obtains the compression override setting of this proxy.
             * @return The compression override setting. If nullopt is returned, no override is set. Otherwise, true
             * if compression is enabled, false otherwise.
             */
            ice_getCompress():boolean;

            /**
             * Obtains a proxy that is identical to this proxy, except for its connection timeout setting
             * which overrides the timeot setting from the proxy endpoints.
             * @param timeout The connection timeout override for the proxy (in milliseconds).
             * @return A proxy with the specified timeout override.
             */
            ice_timeout(timeout:number):this;

            /**
             * Obtains the timeout override of this proxy.
             * @return The timeout override. If nullopt is returned, no override is set. Otherwise, returns
             * the timeout override value.
             */
            ice_getTimeout():number;

            /**
             * Obtains a proxy that is identical to this proxy, except for the router.
             * @param router The router for the new proxy.
             * @return A proxy with the specified router.
             */
            ice_router(router:RouterPrx):this;

            /**
             * Obtains the router for this proxy.
             * @return The router for the proxy. If no router is configured for the proxy, the return value
             * is nil.
             */
            ice_getRouter():RouterPrx;

            /**
             * Obtains a proxy that is identical to this proxy, except for the locator.
             * @param locator The locator for the new proxy.
             * @return A proxy with the specified locator.
             */
            ice_locator(locator:LocatorPrx):this;

            /**
             * Obtains the locator for this proxy.
             * @return The locator for this proxy. If no locator is configured, the return value is nil.
             */
            ice_getLocator():LocatorPrx;

            /**
             * Obtains a proxy that is identical to this proxy, except for the locator cache timeout.
             * @param timeout The new locator cache timeout (in seconds).
             * @return A proxy with the new timeout.
             */
            ice_locatorCacheTimeout(timeout:number):this;

            /**
             * Obtains the locator cache timeout of this proxy.
             * @return The locator cache timeout value (in seconds).
             */
            ice_getLocatorCacheTimeout():number;

            /**
             * Obtains a proxy that is identical to this proxy, except for collocation optimization.
             * @param b True if the new proxy enables collocation optimization, false otherwise.
             * @return A proxy with the specified collocation optimization.
             */
            ice_collocationOptimized(b:boolean):this;

            /**
             * Determines whether this proxy uses collocation optimization.
             * @return True if the proxy uses collocation optimization, false otherwise.
             */
            ice_isCollocationOptimized():boolean;

            /**
             * Obtains a proxy that is identical to this proxy, except for the invocation timeout.
             * @param timeout The new invocation timeout (in milliseconds).
             * @return A proxy with the new timeout.
             */
            ice_invocationTimeout(timeout:number):this;

            /**
             * Obtains the invocation timeout of this proxy.
             * @return The invocation timeout value (in milliseconds).
             */
            ice_getInvocationTimeout():number;

            /**
             * Obtains a proxy that is identical to this proxy, except for its connection ID.
             * @param id The connection ID for the new proxy. An empty string removes the
             * connection ID.
             * @return A proxy with the specified connection ID.
             */
            ice_connectionId(connectionId:string):this;

            /**
             * Obtains the connection ID of this proxy.
             * @return The connection ID.
             */
            ice_getConnectionId():string;

            /**
             * Obtains a proxy that is identical to this proxy, except it's a fixed proxy bound
             * the given connection.
             * @param connection The fixed proxy connection.
             * @return A fixed proxy bound to the given connection.
             */
            ice_fixed(conn:Connection):this;

            /**
             * Returns whether this proxy is a fixed proxy.
             *
             * @return True if this is a fixed proxy, false otherwise.
             **/
            ice_isFixed():boolean;

            /**
             * Obtains the Connection for this proxy. If the proxy does not yet have an established connection,
             * it first attempts to create a connection.
             * @return The asynchronous result object for the invocation.
             */
            ice_getConnection():AsyncResult<Connection>;

            /**
             * Obtains the cached Connection for this proxy. If the proxy does not yet have an established
             * connection, it does not attempt to create a connection.
             * @return The cached connection for this proxy, or nil if the proxy does not have
             * an established connection.
             */
            ice_getCachedConnection():Connection;

            /**
             * Obtains a proxy that is identical to this proxy, except for connection caching.
             * @param cache True if the new proxy should cache connections, false otherwise.
             * @return A proxy with the specified caching policy.
             */
            ice_connectionCached(cache:boolean):this;

            /**
             * Determines whether this proxy caches connections.
             * @return True if this proxy caches connections, false otherwise.
             */
            ice_isConnectionCached():boolean;

            /**
             * Flushes any pending batched requests for this communicator.
             * @return The asynchronous result object for the invocation.
             */
            ice_flushBatchRequests():AsyncResult<void>;

            /**
             * Invokes an operation dynamically.
             * @param operation The name of the operation to invoke.
             * @param mode The operation mode (normal or idempotent).
             * @param inParams An encapsulation containing the encoded in-parameters for the operation.
             * @return The asynchronous result object for the invocation .
             */
            ice_invoke(operation:string, mode:OperationMode, inEncaps:Uint8Array):AsyncResult<[]>;

            /**
             * Compare two proxies for equality
             * @param rhs The proxy to compare with this proxy
             * @returns True if the passed proxy have the same reference than this proxy.
             */
            equals(rhs:any):boolean;

            /**
             * Downcasts a proxy without confirming the target object's type via a remote invocation.
             * @param prx The target proxy.
             * @return A proxy with the requested type.
             */
            static uncheckedCast(prx:ObjectPrx, facet?:string):ObjectPrx;

            /**
             * Downcasts a proxy after confirming the target object's type via a remote invocation.
             * @param prx The target proxy.
             * @param facet A facet name.
             * @param context The context map for the invocation.
             * @return A proxy with the requested type and facet, or nil if the target proxy is nil or the target
             * object does not support the requested type.
             */
            static checkedCast(prx:ObjectPrx, facet?:string, contex?:Map<string, string>):AsyncResult<ObjectPrx>;
        }
    }
}
