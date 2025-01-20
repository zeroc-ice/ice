// Copyright (c) ZeroC, Inc.

declare module "ice" {
    namespace Ice {
        /**
         * The object adapter provides an up-call interface from the Ice run time to the implementation of Ice objects.
         *
         * The object adapter is responsible for receiving requests from endpoints, and for mapping between servants,
         * identities, and proxies.
         *
         * @see {@link Communicator}
         * @see {@link ServantLocator}
         */
        interface ObjectAdapter {
            /**
             * Get the name of this object adapter.
             *
             * @returns This object adapter's name.
             */
            getName(): string;

            /**
             * Get the communicator this object adapter belongs to.
             *
             * @returns This object adapter's communicator.
             *
             * @see {@link Communicator}
             */
            getCommunicator(): Ice.Communicator;

            /**
             * Destroys the object adapter and cleans up all resources held by the object adapter.
             *
             * @see {@link Communicator#destroy}
             */
            destroy(): void;

            /**
             *  Install a middleware in this object adapter.
             *
             * @param middleware The middleware to install.
             * @returns This object adapter.
             * @throws `Error` Thrown if the object adapter's dispatch pipeline has already been
             * created. This creation typically occurs the first time the object adapter dispatches an incoming request.
             */
            use(middleware: (next: Ice.Object) => Ice.Object): ObjectAdapter;

            /**
             * Add a servant to this object adapter's Active Servant Map. Note that one servant can implement several
             * Ice objects by registering the servant with multiple identities. Adding a servant with an identity that
             * is in the map already throws {@link AlreadyRegisteredException}.
             *
             * @param servant The servant to add.
             * @param id The identity of the Ice object that is implemented by the servant.
             * @returns A proxy that matches the given identity and this object adapter.
             *
             * @see {@link Identity}
             * @see {@link ObjectAdapter#addFacet}
             * @see {@link ObjectAdapter#addWithUUID}
             * @see {@link ObjectAdapter#remove}
             * @see {@link ObjectAdapter#find}
             */
            add(servant: Ice.Object, id: Identity): Ice.ObjectPrx;

            /**
             * Like {@link ObjectAdapter#add}, but with a facet.
             *
             * Calling `add(servant, id)` is equivalent to calling {@link ObjectAdapter#addFacet} with an empty facet.
             *
             * @param servant The servant to add.
             * @param id The identity of the Ice object that is implemented by the servant.
             * @param facet The facet. An empty facet means the default facet.
             * @returns A proxy that matches the given identity, facet, and this object adapter.
             *
             * @see {@link Identity}
             * @see {@link ObjectAdapter#add}
             * @see {@link ObjectAdapter#addFacetWithUUID}
             * @see {@link ObjectAdapter#removeFacet}
             * @see {@link ObjectAdapter#findFacet}
             */
            addFacet(servant: Ice.Object, id: Identity, facet: string): Ice.ObjectPrx;

            /**
             * Add a servant to this object adapter's Active Servant Map, using an automatically generated UUID as its
             * identity.
             *
             * The generated UUID identity can be accessed using the {@link ObjectPrx#ice_getIdentity} operation.
             * @param servant The servant to add.
             * @returns A proxy that matches the generated UUID identity and this object adapter.
             *
             * @see {@link Identity}
             * @see {@link ObjectAdapter#add}
             * @see {@link ObjectAdapter#addFacetWithUUID}
             * @see {@link ObjectAdapter#remove}
             * @see {@link ObjectAdapter#find}
             */
            addWithUUID(servant: Ice.Object): Ice.ObjectPrx;

            /**
             * Like {@link ObjectAdapter#addWithUUID}, but with a facet.
             *
             * Calling `addWithUUID(servant)` is equivalent to calling `addFacetWithUUID` with an empty facet.
             *
             * @param servant The servant to add.
             * @param facet The facet. An empty facet means the default facet.
             * @returns A proxy that matches the generated UUID identity, facet, and this object adapter.
             *
             * @see {@link Identity}
             * @see {@link ObjectAdapter#addFacet}
             * @see {@link ObjectAdapter#addWithUUID}
             * @see {@link ObjectAdapter#removeFacet}
             * @see {@link ObjectAdapter#findFacet}
             */
            addFacetWithUUID(servant: Ice.Object, facet: string): Ice.ObjectPrx;

            /**
             * Add a default servant to handle requests for a specific category.
             *
             * Adding a default servant for a category for which a default servant is already registered throws
             * {@link AlreadyRegisteredException}. To dispatch operation calls on servants, the object adapter tries to
             * find a servant for a given Ice object identity and facet in the following order:
             * <ol>
             * <li>The object adapter tries to find a servant for the identity and facet in the Active Servant Map.</li>
             * <li>If no servant has been found in the Active Servant Map, the object adapter tries to find a default servant
             * for the category component of the identity.</li>
             * <li>If no servant has been found by any of the preceding steps, the object adapter tries to find a default
             * servant for an empty category, regardless of the category contained in the identity.</li>
             * <li>If no servant has been found by any of the preceding steps, the object adapter gives up and the caller
             * receives {@link ObjectNotExistException} or {@link FacetNotExistException}.</li>
             * </ol>
             *
             * @param servant The default servant.
             * @param category The category for which the default servant is registered. An empty category means it will
             * handle all categories.
             *
             * @see {@link ObjectAdapter#removeDefaultServant}
             * @see {@link ObjectAdapter#findDefaultServant}
             */
            addDefaultServant(servant: Ice.Object, category: string): void;

            /**
             * Remove a servant (that is, the default facet) from the object adapter's Active Servant Map.
             * @param id The identity of the Ice object that is implemented by the servant. If the servant implements multiple
             * Ice objects, {@link ObjectAdapter#remove} has to be called for all those Ice objects. Removing an identity that is not in
             * the map throws {@link NotRegisteredException}.
             * @returns The removed servant.
             * @see {@link Identity}
             * @see {@link ObjectAdapter#add}
             * @see {@link ObjectAdapter#addWithUUID}
             */
            remove(id: Identity): Ice.Object;

            /**
             * Like {@link ObjectAdapter#remove}, but with a facet. Calling <code>remove(id)</code> is equivalent to calling
             * {@link ObjectAdapter#removeFacet} with an empty facet.
             * @param id The identity of the Ice object that is implemented by the servant.
             * @param facet The facet. An empty facet means the default facet.
             * @returns The removed servant.
             * @see {@link Identity}
             * @see {@link ObjectAdapter#addFacet}
             * @see {@link ObjectAdapter#addFacetWithUUID}
             */
            removeFacet(id: Identity, facet: string): Ice.Object;

            /**
             * Remove all facets with the given identity from the Active Servant Map. The operation completely removes the Ice
             * object, including its default facet. Removing an identity that is not in the map throws
             * {@link NotRegisteredException}.
             * @param id The identity of the Ice object to be removed.
             * @returns A collection containing all the facet names and servants of the removed Ice object.
             * @see {@link ObjectAdapter#remove}
             * @see {@link ObjectAdapter#removeFacet}
             */
            removeAllFacets(id: Identity): Map<string, Ice.Object>;

            /**
             * Remove the default servant for a specific category. Attempting to remove a default servant for a category that
             * is not registered throws {@link NotRegisteredException}.
             * @param category The category of the default servant to remove.
             * @returns The default servant.
             * @see {@link ObjectAdapter#addDefaultServant}
             * @see {@link ObjectAdapter#findDefaultServant}
             */
            removeDefaultServant(category: string): Ice.Object;

            /**
             * Look up a servant in this object adapter's Active Servant Map by the identity of the Ice object it implements.
             * <p class="Note">This operation only tries to look up a servant in the Active Servant Map. It does not attempt
             * to find a servant by using any installed {@link ServantLocator}.
             * @param id The identity of the Ice object for which the servant should be returned.
             * @returns The servant that implements the Ice object with the given identity, or null if no such servant has been
             * found.
             *
             * @see {@link ObjectAdapter#findFacet}
             * @see {@link ObjectAdapter#findByProxy}
             */
            find(id: Identity): Ice.Object;

            /**
             * Like {@link ObjectAdapter#find}, but with a facet.
             *
             * Calling `find(id)` is equivalent to calling {@link ObjectAdapter#findFacet} with an empty facet.
             *
             * @param id The identity of the Ice object for which the servant should be returned.
             * @param facet The facet. An empty facet means the default facet.
             * @returns The servant that implements the Ice object with the given identity and facet, or null if no such
             * servant has been found.
             *
             * @see {@link ObjectAdapter#findByProxy}
             */
            findFacet(id: Identity, facet: string): Ice.Object;

            /**
             * Find all facets with the given identity in the Active Servant Map.
             * @param id The identity of the Ice object for which the facets should be returned.
             * @returns A collection containing all the facet names and servants that have been found, or an empty map if there
             * is no facet for the given identity.
             * @see {@link ObjectAdapter#find}
             * @see {@link ObjectAdapter#findFacet}
             */
            findAllFacets(id: Identity): Map<string, Ice.Object>;

            /**
             * Look up a servant in this object adapter's Active Servant Map, given a proxy.
             *
             * This operation only tries to lookup a servant in the Active Servant Map. It does not attempt to find a
             * servant by using any installed {@link ServantLocator}.
             *
             * @param proxy The proxy for which the servant should be returned.
             * @returns The servant that matches the proxy, or null if no such servant has been found.
             *
             * @see {@link ObjectAdapter#find}
             * @see {@link ObjectAdapter#findFacet}
             */
            findByProxy(proxy: Ice.ObjectPrx): Ice.Object;

            /**
             * Add a Servant Locator to this object adapter.
             *
             * Adding a servant locator for a category for which a servant locator is already registered throws
             * {@link AlreadyRegisteredException}. To dispatch operation calls on servants, the object adapter tries to
             * find a servant for a given Ice object identity and facet in the following order:
             * <ol>
             * <li>The object adapter tries to find a servant for the identity and facet in the Active Servant Map.</li>
             * <li>If no servant has been found in the Active Servant Map, the object adapter tries to find a servant locator
             * for the category component of the identity. If a locator is found, the object adapter tries to find a servant
             * using this locator.</li>
             * <li>If no servant has been found by any of the preceding steps, the object adapter tries to find a locator for
             * an empty category, regardless of the category contained in the identity. If a locator is found, the object
             * adapter tries to find a servant using this locator.</li>
             * <li>If no servant has been found by any of the preceding steps, the object adapter gives up and throws
             * {@link ObjectNotExistException} or {@link FacetNotExistException}.</li>
             * </ol>
             * Only one locator for the empty category can be installed.
             *
             * @param locator The locator to add.
             * @param category The category for which the Servant Locator can locate servants, or an empty string if the
             * Servant Locator does not belong to any specific category.
             *
             * @see {@link ObjectAdapter#removeServantLocator}
             * @see {@link ObjectAdapter#findServantLocator}
             * @see {@link ServantLocator}
             */
            addServantLocator(locator: Ice.ServantLocator, category: string): void;

            /**
             * Remove a Servant Locator from this object adapter.
             *
             * @param category The category for which the Servant Locator can locate servants, or an empty string if the
             * Servant Locator does not belong to any specific category.
             * @returns The Servant Locator.
             * @throws {@link NotRegisteredException} if no Servant Locator was found for the given category.
             *
             * @see {@link ObjectAdapter#addServantLocator}
             * @see {@link ObjectAdapter#findServantLocator}
             * @see {@link ServantLocator}
             */
            removeServantLocator(category: string): Ice.ServantLocator;

            /**
             * Find a Servant Locator installed with this object adapter.
             *
             * @param category The category for which the Servant Locator can locate servants, or an empty string if the
             * Servant Locator does not belong to any specific category.
             * @returns The Servant Locator, or `null` if no Servant Locator was found for the given category.
             *
             * @see {@link ObjectAdapter#addServantLocator}
             * @see {@link ObjectAdapter#removeServantLocator}
             * @see {@link ServantLocator}
             */
            findServantLocator(category: string): Ice.ServantLocator;

            /**
             * Find the default servant for a specific category.
             *
             * @param category The category of the default servant to find.
             * @returns The default servant or null if no default servant was registered for the category.
             *
             * @see {@link ObjectAdapter#addDefaultServant}
             * @see {@link ObjectAdapter#removeDefaultServant}
             */
            findDefaultServant(category: string): Ice.Object;

            /**
             * Create a proxy for the object with the given identity.
             *
             * If this object adapter is configured with an adapter id, the return value is an indirect proxy that
             * refers to the adapter id. If a replica group id is also defined, the return value is an indirect proxy
             * that refers to the replica group id. Otherwise, if no adapter id is defined, the return value is a
             * direct proxy containing this object adapter's published endpoints.
             *
             * @param id The object's identity.
             * @returns A proxy for the object with the given identity.
             */
            createProxy(id: Identity): Ice.ObjectPrx;

            /**
             * Create a direct proxy for the object with the given identity. The returned proxy contains this object
             * adapter's published endpoints.
             *
             * @param id The object's identity.
             * @returns A proxy for the object with the given identity.
             */
            createDirectProxy(id: Identity): Ice.ObjectPrx;

            /**
             * Get the set of endpoints configured with this object adapter.
             *
             * @returns The set of endpoints.
             */
            getEndpoints(): Endpoint[];

            /**
             * Get the set of endpoints that proxies created by this object adapter will contain.
             * @returns The set of published endpoints.
             * @see {@link Endpoint}
             */
            getPublishedEndpoints(): Endpoint[];

            /**
             * Set of the endpoints that proxies created by this object adapter will contain.
             * @param newEndpoints The new set of endpoints that the object adapter will embed in proxies.
             * @see {@link Endpoint}
             */
            setPublishedEndpoints(newEndpoints: Ice.Endpoint[]): void;
        }
    }
}
