// Copyright (c) ZeroC, Inc.

declare module "ice" {
    namespace Ice {
        /**
         * The base class for all object proxies.
         */
        class ObjectPrx {
            /**
             * Constructs a new ObjectPrx proxy.
             *
             * @param communicator - The communicator for the new proxy.
             * @param proxyString - The string representation of the proxy.
             * @returns The newly constructed proxy.
             * @throws {@link ParseException} - Thrown if the proxyString is not a valid proxy string.
             */
            constructor(communicator: Communicator, proxyString: string);

            /**
             * Constructs a new ObjectPrx proxy from another ObjectPrx.
             *
             * The new proxy is a clone of the provided proxy.
             *
             * @param prx - The proxy to clone.
             * @returns The newly constructed proxy.
             */
            constructor(prx: ObjectPrx);

            /**
             * Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
             *
             * @returns The Slice type ID of the most-derived interface supported by the target object.
             */
            static ice_staticId(): string;

            /**
             * Determines if this object supports a specific Slice interface.
             *
             * @param id - The type ID of the Slice interface to check.
             * @param context - An optional context map for the invocation.
             * @returns An asynchronous result that resolves to `true` if this object supports the interface specified
             *          by `id`, or if it derives from the specified interface. Otherwise, it resolves to `false`.
             */
            ice_isA(id: string, context?: Map<string, string>): AsyncResult<boolean>;

            /**
             * Retrieves the Slice type ID of the most-derived interface supported by the target object of this proxy.
             *
             * @param context - An optional context map for the invocation.
             * @returns An asynchronous result that resolves to the Slice type ID of the most-derived interface
             *          supported by the target object.
             */
            ice_id(context?: Map<string, string>): AsyncResult<string>;

            /**
             * Retrieves the Slice type IDs of all interfaces supported by the target object of this proxy.
             *
             * @param context - An optional context map for the invocation.
             * @returns An asynchronous result that resolves to an array of Slice type IDs representing the interfaces
             *          supported by the target object of this proxy.
             */
            ice_ids(context?: Map<string, string>): AsyncResult<string[]>;

            /**
             * Checks if the target object of this proxy is reachable.
             *
             * @param context - An optional context map for the invocation.
             * @returns An asynchronous result that resolves when the ping operation completes.
             *          If the target object is unreachable, the result is rejected with an error.
             */
            ice_ping(context?: Map<string, string>): AsyncResult<void>;

            /**
             * Retrieves the communicator instance that created this proxy.
             *
             * @returns The communicator instance that created this proxy.
             */
            ice_getCommunicator(): Communicator;

            /**
             * Returns a string representation of this proxy.
             *
             * @returns A string representing this proxy.
             */
            ice_toString(): string;

            /**
             * Creates a new proxy identical to this one, but with a different identity.
             *
             * @param id - The identity for the new proxy.
             * @returns A new proxy with the specified identity.
             */
            ice_identity(id: Identity): this;

            /**
             * Retrieves the identity embedded in this proxy.
             *
             * @returns The identity of the target object associated with this proxy.
             */
            ice_getIdentity(): Identity;

            /**
             * Creates a new proxy identical to this one, but with a different adapter ID.
             *
             * @param id - The adapter ID for the new proxy.
             * @returns A new proxy with the specified adapter ID.
             */
            ice_adapterId(id: string): this;

            /**
             * Retrieves the adapter ID associated with this proxy.
             *
             * @returns The adapter ID. If the proxy does not have an adapter ID, an empty string is returned.
             */
            ice_getAdapterId(): string;

            /**
             * Creates a new proxy identical to this one, but with different endpoints.
             *
             * @param endpoints - The endpoints for the new proxy.
             * @returns A new proxy with the specified endpoints.
             */
            ice_endpoints(endpoints: Endpoint[]): this;

            /**
             * Retrieves the endpoints used by this proxy.
             *
             * @returns The endpoints used by this proxy.
             */
            ice_getEndpoints(): Endpoint[];

            /**
             * Creates a new proxy identical to this one, but with a different endpoint selection policy.
             *
             * @param type - The endpoint selection policy for the new proxy.
             * @returns A new proxy with the specified endpoint selection policy.
             */
            ice_endpointSelection(type: EndpointSelectionType): this;

            /**
             * Retrieves the endpoint selection policy used by this proxy.
             *
             * @returns The endpoint selection policy currently used by this proxy.
             */
            ice_getEndpointSelection(): EndpointSelectionType;

            /**
             * Creates a new proxy identical to this one, but with a different per-proxy context.
             *
             * @param context - The context map for the new proxy.
             * @returns A new proxy with the specified per-proxy context.
             */
            ice_context(context: Map<string, string>): this;

            /**
             * Retrieves the per-proxy context used by this proxy.
             *
             * @returns The per-proxy context currently used by this proxy.
             */
            ice_getContext(): Map<string, string>;

            /**
             * Creates a new proxy identical to this one, but with a different facet.
             *
             * @param facet The facet for the new proxy.
             * @returns A new proxy with the specified facet.
             */
            ice_facet(facet: string): this;

            /**
             * Retrieves the facet used by this proxy.
             *
             * @returns The facet currently used by this proxy. If the proxy uses the default facet, an empty string
             *          is returned.
             */
            ice_getFacet(): string;

            /**
             * Creates a new proxy identical to this one, but configured to use twoway invocations.
             *
             * @returns A new proxy configured to uses twoway invocations.
             */
            ice_twoway(): this;

            /**
             * Determines whether this proxy uses twoway invocations.
             *
             * @returns `true` if this proxy uses twoway invocations, `false` otherwise.
             */
            ice_isTwoway(): boolean;

            /**
             * Creates a new proxy identical to this one, but configured to use oneway invocations.
             *
             * @returns A new proxy configured to use oneway invocations.
             */
            ice_oneway(): this;

            /**
             * Determines whether this proxy uses oneway invocations.
             *
             * @returns `true` if this proxy uses oneway invocations, `false` otherwise.
             */
            ice_isOneway(): boolean;

            /**
             * Creates a new proxy identical to this one, but configured to use batch oneway invocations.
             *
             * @returns A new proxy configured to use batch oneway invocations.
             */
            ice_batchOneway(): this;

            /**
             * Determines whether this proxy uses batch oneway invocations.
             *
             * @returns `true` if this proxy uses batch oneway invocations, `false` otherwise.
             */
            ice_isBatchOneway(): boolean;

            /**
             * Creates a new proxy identical to this one, but configured to use datagram invocations.
             *
             * @returns A new proxy configured to use datagram invocations.
             */
            ice_datagram(): this;

            /**
             * Determines whether this proxy uses datagram invocations.
             *
             * @returns `true` if this proxy uses datagram invocations, `false` otherwise.
             */
            ice_isDatagram(): boolean;

            /**
             * Creates a new proxy identical to this one, but configured to use batch datagram invocations.
             *
             * @returns A new proxy configured to use batch datagram invocations.
             */
            ice_batchDatagram(): this;

            /**
             * Determines whether this proxy uses batch datagram invocations.
             *
             * @returns `true` if this proxy uses batch datagram invocations, `false` otherwise.
             */
            ice_isBatchDatagram(): boolean;

            /**
             * Creates a new proxy identical to this one, but with a different endpoint selection policy based on
             * security.
             *
             * @param secure - If `true`, the new proxy uses only endpoints with secure transport. If `false`, the new
             * proxy uses both secure and insecure endpoints.
             * @returns A new proxy configured with the specified security policy for endpoint selection.
             */
            ice_secure(secure: boolean): this;

            /**
             * Retrieves the encoding version used to marshal request parameters.
             *
             * @returns The encoding version currently used by this proxy.
             */
            ice_getEncodingVersion(): EncodingVersion;

            /**
             * Creates a new proxy identical to this one, but with a different encoding version used to marshal
             * parameters.
             *
             * @param encodingVersion - The encoding version to use for marshaling request parameters.
             * @returns A new proxy configured with the specified encoding version.
             */
            ice_encodingVersion(encodingVersion: EncodingVersion): this;

            /**
             * Checks whether this proxy uses only secure endpoints.
             *
             * @returns `true` if this proxy communicates only via secure endpoints, `false` otherwise.
             */
            ice_isSecure(): boolean;

            /**
             * Creates a new proxy identical to this one, but with a different endpoint security preference.
             *
             * @param secure - If `true`, the new proxy will prefer secure endpoints for invocations and will use
             *                 insecure endpoints only if a secure invocation cannot be made. If `false`, the proxy
             *                  will prefer insecure endpoints over secure ones.
             * @returns A new proxy configured with the specified endpoint security preference.
             */
            ice_preferSecure(secure: boolean): this;

            /**
             * Checks whether this proxy prefers secure endpoints.
             *
             * @returns `true` if the proxy always attempts to invoke via secure endpoints before using insecure
             * endpoints, `false` otherwise.
             */
            ice_isPreferSecure(): boolean;

            /**
             * Creates a new proxy identical to this one, but with a different router.
             *
             * @param router - The router to be used by the new proxy.
             * @returns A new proxy configured with the specified router.
             */
            ice_router(router: RouterPrx): this;

            /**
             * Retrieves the router used by this proxy.
             *
             * @returns The router currently used by this proxy. If no router is configured, `null` is returned.
             */
            ice_getRouter(): RouterPrx;

            /**
             * Creates a new proxy identical to this one, but with a different locator.
             *
             * @param locator - The locator to be used by the new proxy.
             * @returns A new proxy configured with the specified locator.
             */
            ice_locator(locator: LocatorPrx): this;

            /**
             * Retrieves the locator used by this proxy.
             *
             * @returns The locator currently used by this proxy. If no locator is configured, `null` is returned.
             */
            ice_getLocator(): LocatorPrx;

            /**
             * Creates a new proxy identical to this one, but with a different locator cache timeout.
             *
             * @param timeout - The locator cache timeout (in seconds) to be used by the new proxy.
             * @returns A new proxy configured with the specified locator cache timeout.
             */
            ice_locatorCacheTimeout(timeout: number): this;

            /**
             * Retrieves the locator cache timeout used by this proxy.
             *
             * @returns The locator cache timeout value (in seconds) currently used by this proxy.
             */
            ice_getLocatorCacheTimeout(): number;

            /**
             * Creates a new proxy identical to this one, but with a different invocation timeout.
             *
             * @param timeout - The invocation timeout (in milliseconds) to be used by the new proxy.
             * @returns A new proxy configured with the specified invocation timeout.
             */
            ice_invocationTimeout(timeout: number): this;

            /**
             * Retrieves the invocation timeout used by this proxy.
             *
             * @returns The invocation timeout value (in milliseconds) currently used by this proxy.
             */
            ice_getInvocationTimeout(): number;

            /**
             * Creates a new proxy identical to this one, but with a different connection ID.
             *
             * @param connectionId - The connection ID for the new proxy. An empty string removes the connection ID.
             * @returns A new proxy configured with the specified connection ID.
             */
            ice_connectionId(connectionId: string): this;

            /**
             * Retrieves the connection ID used by this proxy.
             *
             * @returns The connection ID currently associated with this proxy.
             */
            ice_getConnectionId(): string;

            /**
             * Creates a new proxy identical to this one, but as a fixed proxy bound to the specified connection.
             *
             * @param connection - The connection to be used by the new fixed proxy.
             * @returns A new fixed proxy bound to the specified connection.
             */
            ice_fixed(connection: Connection): this;

            /**
             * Checks whether this proxy is a fixed proxy.
             *
             * @returns `true` if this proxy is a fixed proxy, `false` otherwise.
             */
            ice_isFixed(): boolean;

            /**
             * Retrieves the connection used by this proxy. If the proxy does not yet have an established connection,
             * it attempts to create one.
             *
             * @returns An asynchronous result that resolves to the connection used by this proxy. If the connection
             *          establishment fails, the promise is rejected with an error.
             */
            ice_getConnection(): AsyncResult<Connection>;

            /**
             * Retrieves the cached connection for this proxy.
             *
             * If the proxy does not yet have an established connection, it does not attempt to create one.
             *
             * @returns The cached connection for this proxy, or `null` if no connection is established.
             */
            ice_getCachedConnection(): Connection;

            /**
             * Creates a new proxy identical to this one, but with a different connection caching configuration.
             *
             * @param cache - The connection caching configuration for the new proxy: `true` if the new proxy should cache
             *                connections, `false` otherwise.
             * @returns A new proxy configured with the specified connection caching policy.
             */
            ice_connectionCached(cache: boolean): this;

            /**
             * Checks whether this proxy caches connections.
             *
             * @returns `true` if this proxy caches connections, `false` otherwise.
             */
            ice_isConnectionCached(): boolean;

            /**
             * Flushes any pending batched requests for this proxy.
             *
             * @returns An asynchronous result that is resolved when the pending batched requests for this proxy are
             * flushed.
             */
            ice_flushBatchRequests(): AsyncResult<void>;

            /**
             * Checks whether the specified object is equal to this object.
             *
             * @param other - The object to compare with.
             * @returns `true` if the specified object is equal to this object, `false` otherwise.
             */
            equals(other: any): boolean;

            /**
             * Downcasts a proxy without confirming the target object's type via a remote invocation.
             *
             * @param prx - The target proxy to be downcast.
             * @param facet - An optional facet to specify a part of the target proxy.
             * @returns A proxy with the requested type.
             */
            static uncheckedCast(prx: ObjectPrx, facet?: string): ObjectPrx;

            /**
             * Downcasts a proxy after confirming the target object's type via a remote invocation.
             *
             * @param prx - The target proxy to be downcast.
             * @param facet - An optional facet name specifying a part of the target proxy.
             * @param context - An optional context map for the invocation.
             * @returns An asynchronous result resolving to a proxy with the requested type and facet, or `null` if the
             *          target object does not support the requested type.
             */
            static checkedCast(prx: ObjectPrx, facet?: string, context?: Map<string, string>): AsyncResult<ObjectPrx>;
        }
    }
}
