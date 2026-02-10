// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
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
             * Returns the Slice type ID associated with this type.
             *
             * @returns The Slice type ID.
             */
            static ice_staticId(): string;

            /**
             * Tests whether this object supports a specific Slice interface.
             *
             * @param typeId - The type ID of the Slice interface to test against.
             * @param context - The request context.
             * @returns An asynchronous result that resolves to `true` if the target object implements the Slice
             * interface specified by `typeId` or implements a derived interface, `false` otherwise.
             */
            ice_isA(typeId: string, context?: Map<string, string>): AsyncResult<boolean>;

            /**
             * Gets the type ID of the most-derived Slice interface supported by this object.
             *
             * @param context - The request context.
             * @returns An asynchronous result that resolves to the Slice type ID of the most-derived interface.
             */
            ice_id(context?: Map<string, string>): AsyncResult<string>;

            /**
             * Gets the Slice interfaces supported by this object as a list of Slice type IDs.
             *
             * @param context - The request context.
             * @returns An asynchronous result that resolves to an array of the Slice type IDs of the interfaces
             * supported by this object, in alphabetical order
             */
            ice_ids(context?: Map<string, string>): AsyncResult<string[]>;

            /**
             * Tests whether the target object of this proxy can be reached.
             *
             * @param context - The request context.
             * @returns An asynchronous result that resolves when the ping operation completes.
             */
            ice_ping(context?: Map<string, string>): AsyncResult<void>;

            /**
             * Gets the communicator that created this proxy.
             *
             * @returns The communicator that created this proxy.
             */
            ice_getCommunicator(): Communicator;

            /**
             * Creates a stringified version of this proxy.
             *
             * @returns A stringified proxy.
             */
            ice_toString(): string;

            /**
             * Creates a proxy that is identical to this proxy, except for the identity.
             *
             * @param id - The identity for the new proxy.
             * @returns A proxy with the new identity.
             */
            ice_identity(id: Identity): this;

            /**
             * Gets the identity embedded in this proxy.
             *
             * @returns The identity of the target object.
             */
            ice_getIdentity(): Identity;

            /**
             * Creates a proxy that is identical to this proxy, except for the adapter ID.
             *
             * @param id - The adapter ID for the new proxy.
             * @returns A proxy with the new adapter ID.
             */
            ice_adapterId(id: string): this;

            /**
             * Gets the adapter ID for this proxy.
             *
             * @returns The adapter ID. If the proxy does not have an adapter ID, the return value is the empty string.
             */
            ice_getAdapterId(): string;

            /**
             * Creates a proxy that is identical to this proxy, except for the endpoints.
             *
             * @param endpoints - The endpoints for the new proxy.
             * @returns A proxy with the new endpoints.
             */
            ice_endpoints(endpoints: Endpoint[]): this;

            /**
             * Gets the endpoints used by this proxy.
             *
             * @returns The endpoints used by this proxy.
             */
            ice_getEndpoints(): Endpoint[];

            /**
             * Creates a proxy that is identical to this proxy, except for the endpoint selection policy.
             *
             * @param type - The new endpoint selection policy.
             * @returns A proxy with the specified endpoint selection policy.
             */
            ice_endpointSelection(type: EndpointSelectionType): this;

            /**
             * Gets the endpoint selection policy for this proxy (randomly or ordered).
             *
             * @returns The endpoint selection policy.
             */
            ice_getEndpointSelection(): EndpointSelectionType;

            /**
             * Creates a proxy that is identical to this proxy, except for the per-proxy context.
             *
             * @param context - The context for the new proxy.
             * @returns A proxy with the new per-proxy context.
             */
            ice_context(context: Map<string, string>): this;

            /**
             * Gets the per-proxy context for this proxy.
             *
             * @returns The per-proxy context.
             */
            ice_getContext(): Map<string, string>;

            /**
             * Creates a proxy that is identical to this proxy, except for the facet.
             *
             * @param facet The facet for the new proxy.
             * @returns A proxy with the new facet.
             */
            ice_facet(facet: string): this;

            /**
             * Gets the facet for this proxy.
             *
             * @returns The facet for this proxy. If the proxy uses the default facet, the return value is the empty
             * string.
             */
            ice_getFacet(): string;

            /**
             * Creates a proxy that is identical to this proxy, but uses twoway invocations.
             *
             * @returns A proxy that uses twoway invocations.
             */
            ice_twoway(): this;

            /**
             * Determines whether this proxy uses twoway invocations.
             *
             * @returns `true` if this proxy uses twoway invocations, `false` otherwise.
             */
            ice_isTwoway(): boolean;

            /**
             * Creates a proxy that is identical to this proxy, but uses oneway invocations.
             *
             * @returns A proxy that uses oneway invocations.
             */
            ice_oneway(): this;

            /**
             * Determines whether this proxy uses oneway invocations.
             *
             * @returns `true` if this proxy uses oneway invocations, `false` otherwise.
             */
            ice_isOneway(): boolean;

            /**
             * Creates a proxy that is identical to this proxy, but uses batch oneway invocations.
             *
             * @returns A proxy that uses batch oneway invocations.
             */
            ice_batchOneway(): this;

            /**
             * Determines whether this proxy uses batch oneway invocations.
             *
             * @returns `true` if this proxy uses batch oneway invocations, `false` otherwise.
             */
            ice_isBatchOneway(): boolean;

            /**
             * Gets the encoding version used to marshal request parameters.
             *
             * @returns The encoding version.
             */
            ice_getEncodingVersion(): EncodingVersion;

            /**
             * Creates a proxy that is identical to this proxy, except for the encoding used to marshal parameters.
             *
             * @param version - The encoding version to use to marshal request parameters.
             * @returns A proxy with the specified encoding version.
             */
            ice_encodingVersion(version: EncodingVersion): this;

            /**
             * Creates a proxy that is identical to this proxy, except for the router.
             *
             * @param router - The router for the new proxy.
             * @returns A proxy with the specified router.
             */
            ice_router(router: RouterPrx | null): this;

            /**
             * Gets the router for this proxy.
             *
             * @returns The router for the proxy. If no router is configured for the proxy, the return value is `null`.
             */
            ice_getRouter(): RouterPrx | null;

            /**
             * Creates a proxy that is identical to this proxy, except for the locator.
             *
             * @param locator - The locator for the new proxy.
             * @returns A proxy with the specified locator.
             */
            ice_locator(locator: LocatorPrx | null): this;

            /**
             * Gets the locator for this proxy.
             *
             * @returns The locator for this proxy. If no locator is configured, the return value is `null`.
             */
            ice_getLocator(): LocatorPrx | null;

            /**
             * Creates a proxy that is identical to this proxy, except for the locator cache timeout.
             *
             * @param timeout - The new locator cache timeout (in seconds).
             * @returns A proxy with the new timeout.
             */
            ice_locatorCacheTimeout(timeout: number): this;

            /**
             * Gets the locator cache timeout of this proxy.
             *
             * @returns The locator cache timeout value.
             */
            ice_getLocatorCacheTimeout(): number;

            /**
             * Creates a proxy that is identical to this proxy, except for the invocation timeout.
             *
             * @param timeout - The new invocation timeout (in milliseconds).
             * @returns A proxy with the new timeout.
             */
            ice_invocationTimeout(timeout: number): this;

            /**
             * Gets the invocation timeout of this proxy.
             *
             * @returns The invocation timeout value.
             */
            ice_getInvocationTimeout(): number;

            /**
             * Creates a proxy that is identical to this proxy, except for its connection ID.
             *
             * @param connectionId - The connection ID for the new proxy. An empty string removes the connection ID.
             * @returns A proxy with the specified connection ID.
             */
            ice_connectionId(connectionId: string): this;

            /**
             * Gets the connection ID of this proxy.
             *
             * @returns The connection ID.
             */
            ice_getConnectionId(): string;

            /**
             * Creates a proxy that is identical to this proxy, except it's a fixed proxy bound to the given connection.
             *
             * @param connection - The fixed proxy connection.
             * @returns A fixed proxy bound to the given connection.
             */
            ice_fixed(connection: Connection): this;

            /**
             * Determines whether this proxy is a fixed proxy.
             *
             * @returns `true` if this proxy is a fixed proxy, `false` otherwise.
             */
            ice_isFixed(): boolean;

            /**
             * Gets the connection ID of this proxy.
             *
             * @returns An asynchronous result that resolves to the connection used by this proxy.
             * @remarks You can call this function to establish a connection or associate the proxy with an existing
             * connection.
             */
            ice_getConnection(): AsyncResult<Connection>;

            /**
             * Gets the cached Connection for this proxy. If the proxy does not yet have an established connection,
             * it does not attempt to create a connection.
             *
             * @returns The cached connection for this proxy, or `null` if the proxy does not have an established
             * connection.
             */
            ice_getCachedConnection(): Connection;

            /**
             * Creates a proxy that is identical to this proxy, except for connection caching.
             *
             * @param cache - `true` if the new proxy should cache connections, `false` otherwise.
             * @returns A proxy with the specified caching policy.
             */
            ice_connectionCached(cache: boolean): this;

            /**
             * Determines whether this proxy caches connections.
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
            equals(other: ObjectPrx | null | undefined): boolean;

            /**
             * Creates a new proxy from an existing proxy.
             *
             * @param prx - The source proxy.
             * @param facet - An optional facet name.
             * @returns A proxy with the requested type and facet, or null if the source proxy is null.
             */
            static uncheckedCast(prx: ObjectPrx, facet?: string): ObjectPrx;

            /**
             * Creates a new proxy from an existing proxy.
             *
             * @param prx - The source proxy.
             * @param facet - An optional facet name.
             * @returns A proxy with the requested type and facet, or null if the source proxy is null.
             */
            static uncheckedCast(prx: ObjectPrx | null, facet?: string): ObjectPrx | null;

            /**
             * Creates a new proxy from an existing proxy after confirming the target object's type via a remote
             * invocation.
             *
             * @param prx - The source proxy.
             * @param facet - An optional facet name.
             * @param context - The request context.
             * @returns An asynchronous result resolving to a proxy with the requested type and facet, or `null` if the
             *          target object does not support the requested type.
             */
            static checkedCast(
                prx: ObjectPrx | null,
                facet?: string,
                context?: Map<string, string>,
            ): AsyncResult<ObjectPrx | null>;
        }
    }
}
