// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * An object adapter is the main server-side Ice API. It has two main purposes:
         *
         * - dispatch requests received over outgoing connections associated with this object adapter, and
         * - maintain a dispatch pipeline and servants that handle the requests (see {@link ObjectAdapter#add},
         *   {@link ObjectAdapter#addDefaultServant}, and {@link ObjectAdapter#use}).
         *
         * @see {@link Communicator#createObjectAdapter}
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
             *  Adds a middleware to the dispatch pipeline of this object adapter.
             *
             * @param middlewareFactory The middleware factory that creates the new middleware when this object adapter
             * creates its dispatch pipeline. A middleware factory is a function that takes an Object (the next element
             * in the dispatch pipeline) and returns a new Object (the middleware you want to install in the pipeline).
             * @returns This object adapter.
             * @throws `Error` Thrown if the object adapter's dispatch pipeline has already been
             * created. This creation typically occurs the first time the object adapter dispatches an incoming request.
             */
            use(middlewareFactory: (next: Ice.Object) => Ice.Object): ObjectAdapter;

            /**
             * Adds a servant to this object adapter's Active Servant Map (ASM).
             * The ASM is a map {identity, facet} -> servant.
             *
             * @param servant The servant to add.
             * @param id The identity of the Ice object that is implemented by the servant.
             * @returns A proxy with the given id, created by this object adapter.
             *
             * @see {@link Identity}
             * @see {@link ObjectAdapter#addFacet}
             * @see {@link ObjectAdapter#addWithUUID}
             * @see {@link ObjectAdapter#remove}
             * @see {@link ObjectAdapter#find}
             */
            add(servant: Ice.Object, id: Identity): Ice.ObjectPrx;

            /**
             * Adds a servant to this object adapter's Active Servant Map (ASM), while specifying a facet.
             * The ASM is a map {identity, facet} -> servant.
             *
             * Calling `add(servant, id)` is equivalent to calling {@link ObjectAdapter#addFacet} with an empty facet.
             *
             * @param servant The servant to add.
             * @param id The identity of the Ice object that is implemented by the servant.
             * @param facet The facet of the Ice object that is implemented by the servant.
             * @returns A proxy with the given id and facet, created by this object adapter.
             */
            addFacet(servant: Ice.Object, id: Identity, facet: string): Ice.ObjectPrx;

            /**
             * Adds a servant to this object adapter's Active Servant Map (ASM), using an automatically generated UUID
             * as its identity.
             *
             * The generated UUID identity can be accessed using the {@link ObjectPrx#ice_getIdentity} operation.
             * @param servant The servant to add.
             * @returns A proxy with the generated UUID identity created by this object adapter.
             */
            addWithUUID(servant: Ice.Object): Ice.ObjectPrx;

            /**
             * Adds a servant to this object adapter's Active Servant Map (ASM), using an automatically generated UUID
             * as its identity. Also specifies a facet.
             *
             * @param servant The servant to add.
             * @param facet The facet of the Ice object that is implemented by the servant.
             * @returns A proxy with the generated UUID identity and the specified facet.
             */
            addFacetWithUUID(servant: Ice.Object, facet: string): Ice.ObjectPrx;

            /**
             * Adds a default servant to handle requests for a specific category. When an object adapter dispatches an
             * incoming request, it tries to find a servant for the identity and facet carried by the request in the
             * following order:
             *
             *  - The object adapter tries to find a servant for the identity and facet in the Active Servant Map.
             *  - If this fails, the object adapter tries to find a default servant for the category component of the
             *    identity.
             *  - If this fails, the object adapter tries to find a default servant for the empty category, regardless of
             *    the category contained in the identity.
             *  - If this fails, the object adapter tries to find a servant locator for the category component of the
             *    identity. If there is no such servant locator, the object adapter tries to find a servant locator for the
             *    empty category.
             *  - If a servant locator is found, the object adapter tries to find a servant using this servant locator.
             *  - If all the previous steps fail, the object adapter gives up and the caller receives an
             *    ObjectNotExistException or a FacetNotExistException.
             *
             * @param servant The default servant to add.
             * @param category The category for which the default servant is registered. The empty category means it will
             * handle all categories.
             * @throws {@link AlreadyRegisteredException} Thrown when a default servant with the same category is
             * already registered.
             */
            addDefaultServant(servant: Ice.Object, category: string): void;

            /**
             * Removes a servant from the object adapter's Active Servant Map.
             * @param id The identity of the Ice object that is implemented by the servant.
             * @returns The removed servant.
             * @throws {@link NotRegisteredException} Thrown when no servant with the given identity is registered.
             */
            remove(id: Identity): Ice.Object;

            /**
             * Removes a servant from the object adapter's Active Servant Map, while specifying a facet.
             * @param id The identity of the Ice object that is implemented by the servant.
             * @param facet The facet. An empty facet means the default facet.
             * @returns The removed servant.
             * @throws {@link NotRegisteredException} Thrown when no servant with the given identity and facet is
             * registered.
             */
            removeFacet(id: Identity, facet: string): Ice.Object;

            /**
             * Removes all facets with the given identity from the Active Servant Map. The function completely removes
             * the Ice object, including its default facet.
             * @param id The identity of the Ice object to be removed.
             * @returns A collection containing all the facet names and servants of the removed Ice object.
             * @throws {@link NotRegisteredException} Thrown when no servant with the given identity is registered.
             */
            removeAllFacets(id: Identity): Map<string, Ice.Object>;

            /**
             * Removes the default servant for a specific category.
             * @param category The category of the default servant to remove.
             * @returns The default servant.
             * @throws {@link NotRegisteredException} Thrown when no default servant is registered for the given
             * category.
             */
            removeDefaultServant(category: string): Ice.Object;

            /**
             * Looks up a servant..
             * @param id The identity of an Ice object.
             * @returns The servant that implements the Ice object with the given identity, or null if no such servant
             * has been found.
             * @remarks This function only tries to find the servant in the ASM and among the default servants. It does
             * not attempt to locate a servant using servant locators.
             */
            find(id: Identity): Ice.Object;

            /**
             * Looks up a servant with an identity and facet.
             *
             * @param id The identity of an Ice object.
             * @param facet The facet of an Ice object. An empty facet means the default facet.
             * @returns The servant that implements the Ice object with the given identity and facet, or null if no
             * such servant has been found.
             * @remark This function only tries to find the servant in the ASM and among the default servants. It does
             * not attempt to locate a servant using servant locators.
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
             * Looks up a servant with an identity and a facet. It's equivalent to calling #findFacet.
             *
             * @param proxy The proxy that provides the identity and facet to search.
             * @returns The servant that matches the identity and facet carried by the proxy, or null if no such
             * servant has been found.
             */
            findByProxy(proxy: Ice.ObjectPrx): Ice.Object;

            /**
             * Adds a ServantLocator to this object adapter for a specific category.
             *
             * @param locator The servant locator to add.
             * @param category The category. The empty category means the locator handles all categories.
             * @throws {@link AlreadyRegisteredException} Thrown when a servant locator with the same category is
             * already registered.
             */
            addServantLocator(locator: Ice.ServantLocator, category: string): void;

            /**
             * Removes a ServantLocator from this object adapter.
             *
             * @param category The category.
             * @returns The servant locator.
             * @throws {@link NotRegisteredException}Thrown when no ServantLocator with the given category is
             * registered.
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
             * Finds a ServantLocator registered with this object adapter.
             *
             * @param category The category.
             * @returns The servant locator, or null if not found.
             */
            findDefaultServant(category: string): Ice.Object;

            /**
             * Creates a proxy from an Ice identity. If this object adapter is configured with an adapter ID, the proxy
             * is an indirect proxy that refers to this adapter ID. If a replica group ID is also defined, the proxy is an
             * indirect proxy that refers to this replica group ID. Otherwise, the proxy is a direct proxy containing this
             * object adapter's published endpoints.
             *
             * @param id The object's identity.
             * @returns A proxy for the object with the given identity.
             */
            createProxy(id: Identity): Ice.ObjectPrx;

            /**
             * Creates a direct proxy from an Ice identity.
             *
             * @param id The object's identity.
             * @returns A proxy for the object with the given identity.
             */
            createDirectProxy(id: Identity): Ice.ObjectPrx;

            /**
             * Gets the set of endpoints configured on this object adapter.
             *
             * @returns The set of endpoints.
             */
            getEndpoints(): Endpoint[];

            /**
             * Gets the set of endpoints that proxies created by this object adapter will contain.
             * @returns The set of published endpoints.
             * @see {@link Endpoint}
             */
            getPublishedEndpoints(): Endpoint[];

            /**
             * Sets the endpoints that proxies created by this object adapter will contain.
             * @param newEndpoints The new set of endpoints that the object adapter will embed in proxies.
             * @throws `Error` Thrown when the `newEndpoints` is empty or this adapter is associated with a router.
             */
            setPublishedEndpoints(newEndpoints: Ice.Endpoint[]): void;
        }
    }
}
