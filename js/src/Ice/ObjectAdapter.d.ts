//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice" {
    namespace Ice {
        /**
         * The object adapter provides an up-call interface from the Ice run time to the implementation of Ice objects. The
         * object adapter is responsible for receiving requests from endpoints, and for mapping between servants, identities,
         * and proxies.
         * @see Communicator
         * @see ServantLocator
         */
        interface ObjectAdapter {
            /**
             * Get the name of this object adapter.
             * @return This object adapter's name.
             */
            getName(): string;
            /**
             * Get the communicator this object adapter belongs to.
             * @return This object adapter's communicator.
             * @see Communicator
             */
            getCommunicator(): Ice.Communicator;
            /**
             * Activate all endpoints that belong to this object adapter. After activation, the object adapter can dispatch
             * requests received through its endpoints.
             * @return @returns The asynchronous result object for the invocation.
             * @see ObjectAdapter#hold
             * @see ObjectAdapter#deactivate
             */
            activate(): AsyncResultBase<void>;
            /**
             * Temporarily hold receiving and dispatching requests. The object adapter can be reactivated with the
             * {@link ObjectAdapter#activate} operation. <p class="Note"> Holding is not immediate, i.e., after {@link ObjectAdapter#hold} returns, the
             * object adapter might still be active for some time. You can use {@link ObjectAdapter#waitForHold} to wait until holding is
             * complete.
             * @see ObjectAdapter#activate
             * @see ObjectAdapter#deactivate
             * @see ObjectAdapter#waitForHold
             */
            hold(): void;
            /**
             * Wait until the object adapter holds requests. Calling {@link ObjectAdapter#hold} initiates holding of requests, and
             * {@link ObjectAdapter#waitForHold} only returns when holding of requests has been completed.
             * @return @returns The asynchronous result object for the invocation.
             * @see ObjectAdapter#hold
             * @see ObjectAdapter#waitForDeactivate
             * @see Communicator#waitForShutdown
             */
            waitForHold(): AsyncResultBase<void>;
            /**
             * Deactivate all endpoints that belong to this object adapter. After deactivation, the object adapter stops
             * receiving requests through its endpoints. Object adapters that have been deactivated must not be reactivated
             * again, and cannot be used otherwise. Attempts to use a deactivated object adapter raise
             * {@link ObjectAdapterDeactivatedException} however, attempts to {@link ObjectAdapter#deactivate} an already deactivated
             * object adapter are ignored and do nothing. Once deactivated, it is possible to destroy the adapter to clean up
             * resources and then create and activate a new adapter with the same name.
             * <p class="Note"> After {@link ObjectAdapter#deactivate} returns, no new requests are processed by the object adapter.
             * However, requests that have been started before {@link ObjectAdapter#deactivate} was called might still be active. You can
             * use {@link ObjectAdapter#waitForDeactivate} to wait for the completion of all requests for this object adapter.
             * @return @returns The asynchronous result object for the invocation.
             * @see ObjectAdapter#activate
             * @see ObjectAdapter#hold
             * @see ObjectAdapter#waitForDeactivate
             * @see Communicator#shutdown
             */
            deactivate(): AsyncResultBase<void>;
            /**
             * Wait until the object adapter has deactivated. Calling {@link ObjectAdapter#deactivate} initiates object adapter
             * deactivation, and {@link ObjectAdapter#waitForDeactivate} only returns when deactivation has been completed.
             * @return The asynchronous result object for the invocation.
             * @see ObjectAdapter#deactivate
             * @see ObjectAdapter#waitForHold
             * @see Communicator#waitForShutdown
             */
            waitForDeactivate(): AsyncResultBase<void>;
            /**
             * Check whether object adapter has been deactivated.
             * @return Whether adapter has been deactivated.
             * @see Communicator#shutdown
             */
            isDeactivated(): boolean;
            /**
             * Destroys the object adapter and cleans up all resources held by the object adapter. If the object adapter has
             * not yet been deactivated, destroy implicitly initiates the deactivation and waits for it to finish. Subsequent
             * calls to destroy are ignored. Once destroy has returned, it is possible to create another object adapter with
             * the same name.
             * @return The asynchronous result object for the invocation.
             * @see ObjectAdapter#deactivate
             * @see ObjectAdapter#waitForDeactivate
             * @see Communicator#destroy
             */
            destroy(): AsyncResultBase<void>;

            /**
             *  Install a middleware in this object adapter.
             *
             * @param middleware The middleware to install.
             * @return This object adapter.
             * @throws Error Thrown if the object adapter's dispatch pipeline has already been
             * created. This creation typically occurs the first time the object adapter dispatches an incoming request.
             */
            use(middleware: (next: Ice.Object) => Ice.Object): ObjectAdapter;

            /**
             * Add a servant to this object adapter's Active Servant Map. Note that one servant can implement several Ice
             * objects by registering the servant with multiple identities. Adding a servant with an identity that is in the
             * map already throws {@link AlreadyRegisteredException}.
             * @param servant The servant to add.
             * @param id The identity of the Ice object that is implemented by the servant.
             * @return A proxy that matches the given identity and this object adapter.
             * @see Identity
             * @see ObjectAdapter#addFacet
             * @see ObjectAdapter#addWithUUID
             * @see ObjectAdapter#remove
             * @see ObjectAdapter#find
             */
            add(servant: Ice.Object, id: Identity): Ice.ObjectPrx;
            /**
             * Like {@link ObjectAdapter#add}, but with a facet. Calling <code>add(servant, id)</code> is equivalent to calling
             * {@link ObjectAdapter#addFacet} with an empty facet.
             * @param servant The servant to add.
             * @param id The identity of the Ice object that is implemented by the servant.
             * @param facet The facet. An empty facet means the default facet.
             * @return A proxy that matches the given identity, facet, and this object adapter.
             * @see Identity
             * @see ObjectAdapter#add
             * @see ObjectAdapter#addFacetWithUUID
             * @see ObjectAdapter#removeFacet
             * @see ObjectAdapter#findFacet
             */
            addFacet(servant: Ice.Object, id: Identity, facet: string): Ice.ObjectPrx;
            /**
             * Add a servant to this object adapter's Active Servant Map, using an automatically generated UUID as its
             * identity. Note that the generated UUID identity can be accessed using the proxy's <code>ice_getIdentity</code>
             * operation.
             * @param servant The servant to add.
             * @return A proxy that matches the generated UUID identity and this object adapter.
             * @see Identity
             * @see ObjectAdapter#add
             * @see ObjectAdapter#addFacetWithUUID
             * @see ObjectAdapter#remove
             * @see ObjectAdapter#find
             */
            addWithUUID(servant: Ice.Object): Ice.ObjectPrx;
            /**
             * Like {@link ObjectAdapter#addWithUUID}, but with a facet. Calling <code>addWithUUID(servant)</code> is equivalent to calling
             * {@link ObjectAdapter#addFacetWithUUID} with an empty facet.
             * @param servant The servant to add.
             * @param facet The facet. An empty facet means the default facet.
             * @return A proxy that matches the generated UUID identity, facet, and this object adapter.
             * @see Identity
             * @see ObjectAdapter#addFacet
             * @see ObjectAdapter#addWithUUID
             * @see ObjectAdapter#removeFacet
             * @see ObjectAdapter#findFacet
             */
            addFacetWithUUID(servant: Ice.Object, facet: string): Ice.ObjectPrx;
            /**
             * Add a default servant to handle requests for a specific category. Adding a default servant for a category for
             * which a default servant is already registered throws {@link AlreadyRegisteredException}. To dispatch operation
             * calls on servants, the object adapter tries to find a servant for a given Ice object identity and facet in the
             * following order:
             * <ol>
             * <li>The object adapter tries to find a servant for the identity and facet in the Active Servant Map.</li>
             * <li>If no servant has been found in the Active Servant Map, the object adapter tries to find a default servant
             * for the category component of the identity.</li>
             * <li>If no servant has been found by any of the preceding steps, the object adapter tries to find a default
             * servant for an empty category, regardless of the category contained in the identity.</li>
             * <li>If no servant has been found by any of the preceding steps, the object adapter gives up and the caller
             * receives {@link ObjectNotExistException} or {@link FacetNotExistException}.</li>
             * </ol>
             * @param servant The default servant.
             * @param category The category for which the default servant is registered. An empty category means it will
             * handle all categories.
             * @see ObjectAdapter#removeDefaultServant
             * @see ObjectAdapter#findDefaultServant
             */
            addDefaultServant(servant: Ice.Object, category: string): void;
            /**
             * Remove a servant (that is, the default facet) from the object adapter's Active Servant Map.
             * @param id The identity of the Ice object that is implemented by the servant. If the servant implements multiple
             * Ice objects, {@link ObjectAdapter#remove} has to be called for all those Ice objects. Removing an identity that is not in
             * the map throws {@link NotRegisteredException}.
             * @return The removed servant.
             * @see Identity
             * @see ObjectAdapter#add
             * @see ObjectAdapter#addWithUUID
             */
            remove(id: Identity): Ice.Object;
            /**
             * Like {@link ObjectAdapter#remove}, but with a facet. Calling <code>remove(id)</code> is equivalent to calling
             * {@link ObjectAdapter#removeFacet} with an empty facet.
             * @param id The identity of the Ice object that is implemented by the servant.
             * @param facet The facet. An empty facet means the default facet.
             * @return The removed servant.
             * @see Identity
             * @see ObjectAdapter#addFacet
             * @see ObjectAdapter#addFacetWithUUID
             */
            removeFacet(id: Identity, facet: string): Ice.Object;
            /**
             * Remove all facets with the given identity from the Active Servant Map. The operation completely removes the Ice
             * object, including its default facet. Removing an identity that is not in the map throws
             * {@link NotRegisteredException}.
             * @param id The identity of the Ice object to be removed.
             * @return A collection containing all the facet names and servants of the removed Ice object.
             * @see ObjectAdapter#remove
             * @see ObjectAdapter#removeFacet
             */
            removeAllFacets(id: Identity): Map<string, Ice.Object>;
            /**
             * Remove the default servant for a specific category. Attempting to remove a default servant for a category that
             * is not registered throws {@link NotRegisteredException}.
             * @param category The category of the default servant to remove.
             * @return The default servant.
             * @see ObjectAdapter#addDefaultServant
             * @see ObjectAdapter#findDefaultServant
             */
            removeDefaultServant(category: string): Ice.Object;
            /**
             * Look up a servant in this object adapter's Active Servant Map by the identity of the Ice object it implements.
             * <p class="Note">This operation only tries to look up a servant in the Active Servant Map. It does not attempt
             * to find a servant by using any installed {@link ServantLocator}.
             * @param id The identity of the Ice object for which the servant should be returned.
             * @return The servant that implements the Ice object with the given identity, or null if no such servant has been
             * found.
             * @see Identity
             * @see ObjectAdapter#findFacet
             * @see ObjectAdapter#findByProxy
             */
            find(id: Identity): Ice.Object;
            /**
             * Like {@link ObjectAdapter#find}, but with a facet. Calling <code>find(id)</code> is equivalent to calling {@link ObjectAdapter#findFacet}
             * with an empty facet.
             * @param id The identity of the Ice object for which the servant should be returned.
             * @param facet The facet. An empty facet means the default facet.
             * @return The servant that implements the Ice object with the given identity and facet, or null if no such
             * servant has been found.
             * @see Identity
             * @see ObjectAdapter#find
             * @see ObjectAdapter#findByProxy
             */
            findFacet(id: Identity, facet: string): Ice.Object;
            /**
             * Find all facets with the given identity in the Active Servant Map.
             * @param id The identity of the Ice object for which the facets should be returned.
             * @return A collection containing all the facet names and servants that have been found, or an empty map if there
             * is no facet for the given identity.
             * @see ObjectAdapter#find
             * @see ObjectAdapter#findFacet
             */
            findAllFacets(id: Identity): Map<string, Ice.Object>;
            /**
             * Look up a servant in this object adapter's Active Servant Map, given a proxy.
             * <p class="Note">This operation only tries to lookup a servant in the Active Servant Map. It does not attempt to
             * find a servant by using any installed {@link ServantLocator}.
             * @param proxy The proxy for which the servant should be returned.
             * @return The servant that matches the proxy, or null if no such servant has been found.
             * @see ObjectAdapter#find
             * @see ObjectAdapter#findFacet
             */
            findByProxy(proxy: Ice.ObjectPrx): Ice.Object;
            /**
             * Add a Servant Locator to this object adapter. Adding a servant locator for a category for which a servant
             * locator is already registered throws {@link AlreadyRegisteredException}. To dispatch operation calls on
             * servants, the object adapter tries to find a servant for a given Ice object identity and facet in the following
             * order:
             * <ol>
             * <li>The object adapter tries to find a servant for the identity and facet in the Active Servant Map.</li>
             * <li>If no servant has been found in the Active Servant Map, the object adapter tries to find a servant locator
             * for the category component of the identity. If a locator is found, the object adapter tries to find a servant
             * using this locator.</li>
             * <li>If no servant has been found by any of the preceding steps, the object adapter tries to find a locator for
             * an empty category, regardless of the category contained in the identity. If a locator is found, the object
             * adapter tries to find a servant using this locator.</li>
             * <li>If no servant has been found by any of the preceding steps, the object adapter gives up and the caller
             * receives {@link ObjectNotExistException} or {@link FacetNotExistException}.</li>
             * </ol>
             * <p class="Note">Only one locator for the empty category can be installed.
             * @param locator The locator to add.
             * @param category The category for which the Servant Locator can locate servants, or an empty string if the
             * Servant Locator does not belong to any specific category.
             * @see Identity
             * @see ObjectAdapter#removeServantLocator
             * @see ObjectAdapter#findServantLocator
             * @see ServantLocator
             */
            addServantLocator(locator: Ice.ServantLocator, category: string): void;
            /**
             * Remove a Servant Locator from this object adapter.
             * @param category The category for which the Servant Locator can locate servants, or an empty string if the
             * Servant Locator does not belong to any specific category.
             * @return The Servant Locator, or throws {@link NotRegisteredException} if no Servant Locator was found for the
             * given category.
             * @see Identity
             * @see ObjectAdapter#addServantLocator
             * @see ObjectAdapter#findServantLocator
             * @see ServantLocator
             */
            removeServantLocator(category: string): Ice.ServantLocator;
            /**
             * Find a Servant Locator installed with this object adapter.
             * @param category The category for which the Servant Locator can locate servants, or an empty string if the
             * Servant Locator does not belong to any specific category.
             * @return The Servant Locator, or null if no Servant Locator was found for the given category.
             * @see Identity
             * @see ObjectAdapter#addServantLocator
             * @see ObjectAdapter#removeServantLocator
             * @see ServantLocator
             */
            findServantLocator(category: string): Ice.ServantLocator;
            /**
             * Find the default servant for a specific category.
             * @param category The category of the default servant to find.
             * @return The default servant or null if no default servant was registered for the category.
             * @see ObjectAdapter#addDefaultServant
             * @see ObjectAdapter#removeDefaultServant
             */
            findDefaultServant(category: string): Ice.Object;
            /**
             * Create a proxy for the object with the given identity. If this object adapter is configured with an adapter id,
             * the return value is an indirect proxy that refers to the adapter id. If a replica group id is also defined, the
             * return value is an indirect proxy that refers to the replica group id. Otherwise, if no adapter id is defined,
             * the return value is a direct proxy containing this object adapter's published endpoints.
             * @param id The object's identity.
             * @return A proxy for the object with the given identity.
             * @see Identity
             */
            createProxy(id: Identity): Ice.ObjectPrx;
            /**
             * Create a direct proxy for the object with the given identity. The returned proxy contains this object adapter's
             * published endpoints.
             * @param id The object's identity.
             * @return A proxy for the object with the given identity.
             * @see Identity
             */
            createDirectProxy(id: Identity): Ice.ObjectPrx;
            /**
             * Create an indirect proxy for the object with the given identity. If this object adapter is configured with an
             * adapter id, the return value refers to the adapter id. Otherwise, the return value contains only the object
             * identity.
             * @param id The object's identity.
             * @return A proxy for the object with the given identity.
             * @see Identity
             */
            createIndirectProxy(id: Identity): Ice.ObjectPrx;
            /**
             * Set an Ice locator for this object adapter. By doing so, the object adapter will register itself with the
             * locator registry when it is activated for the first time. Furthermore, the proxies created by this object
             * adapter will contain the adapter identifier instead of its endpoints. The adapter identifier must be configured
             * using the AdapterId property.
             * @param loc The locator used by this object adapter.
             * @see ObjectAdapter#createDirectProxy
             * @see Locator
             * @see LocatorRegistry
             */
            setLocator(loc: LocatorPrx): void;
            /**
             * Get the Ice locator used by this object adapter.
             * @return The locator used by this object adapter, or null if no locator is used by this object adapter.
             * @see Locator
             * @see ObjectAdapter#setLocator
             */
            getLocator(): LocatorPrx;
            /**
             * Get the set of endpoints configured with this object adapter.
             * @return The set of endpoints.
             * @see Endpoint
             */
            getEndpoints(): Endpoint[];
            /**
             * Refresh the set of published endpoints. The run time re-reads the PublishedEndpoints property if it is set and
             * re-reads the list of local interfaces if the adapter is configured to listen on all endpoints. This operation
             * is useful to refresh the endpoint information that is published in the proxies that are created by an object
             * adapter if the network interfaces used by a host changes.
             * @return @returns The asynchronous result object for the invocation.
             */
            refreshPublishedEndpoints(): AsyncResultBase<void>;
            /**
             * Get the set of endpoints that proxies created by this object adapter will contain.
             * @return The set of published endpoints.
             * @see ObjectAdapter#refreshPublishedEndpoints
             * @see Endpoint
             */
            getPublishedEndpoints(): Endpoint[];
            /**
             * Set of the endpoints that proxies created by this object adapter will contain.
             * @param newEndpoints The new set of endpoints that the object adapter will embed in proxies.
             * @see ObjectAdapter#refreshPublishedEndpoints
             * @see Endpoint
             */
            setPublishedEndpoints(newEndpoints: Ice.Endpoint[]): void;
        }
    }
}
