// Copyright (c) ZeroC, Inc.

import Foundation

/// An object adapter is the main server-side Ice API. It has two main purposes:
/// - accept incoming connections from clients and dispatch requests received over these connections
///   (see ``activate()``); and
/// - maintain a dispatch pipeline and servants that handle the requests
///   (see ``add(servant:id:)``, ``addDefaultServant(servant:category:)``, and ``use(_:)``).
///
/// An object adapter can dispatch "bidirectional requests" (requests received over an outgoing connection)
/// and collocated requests (with no connection at all).
public protocol ObjectAdapter: AnyObject, Sendable {

    /// Gets the dispatch pipeline of this object adapter.
    var dispatchPipeline: Dispatcher { get }

    /// Gets the name of this object adapter.
    ///
    /// - Returns: This object adapter's name.
    func getName() -> String

    /// Gets the communicator that created this object adapter.
    ///
    /// - Returns: This object adapter's communicator.
    func getCommunicator() -> Communicator

    /// Starts receiving and dispatching requests received over incoming connections.
    /// When this object adapter is an indirect object adapter configured with a locator proxy,
    /// this method also registers the object adapter's published endpoints with this locator.
    func activate() throws

    /// Stops reading requests from incoming connections. Outstanding dispatches are not affected. The object adapter
    /// can be reactivated with ``activate()``.
    ///
    /// - Remark: This method is provided for backward compatibility with older versions of Ice. Don't use it in
    /// new applications.
    func hold()

    /// Waits until the object adapter is in the holding state (see ``hold()``) and the dispatch of requests received over
    /// incoming connections has completed.
    ///
    /// - Remark: This method is provided for backward compatibility with older versions of Ice. Don't use it in
    /// new applications.
    func waitForHold()

    /// Deactivates this object adapter: stops accepting new connections from clients and closes gracefully all
    /// incoming connections created by this object adapter once all outstanding dispatches have completed. If this
    /// object adapter is indirect, this method also unregisters the object adapter from the locator.
    /// This method does not cancel outstanding dispatches: it lets them execute until completion.
    /// A deactivated object adapter cannot be reactivated again; it can only be destroyed.
    func deactivate()

    /// Waits until ``deactivate()`` is called on this object adapter and all connections accepted by this object adapter are
    /// closed. A connection is closed only after all outstanding dispatches on this connection have completed.
    func waitForDeactivate()

    /// Checks whether or not ``deactivate()`` was called on this object adapter.
    ///
    /// - Returns: `true` if ``deactivate()`` was called on this object adapter, `false` otherwise.
    func isDeactivated() -> Bool

    /// Destroys this object adapter and cleans up all resources associated with it. Once this method has returned,
    /// you can recreate another object adapter with the same name.
    func destroy()

    /// Adds a middleware to the dispatch pipeline of this object adapter.
    /// Middleware are executed in the order they are added.
    /// All middleware must be installed before the first dispatch and execute in the order they are installed.
    ///
    /// - Parameter middlewareFactory: The middleware factory that creates the new middleware when this object adapter
    ///   creates its dispatch pipeline. A middleware factory is a function that takes a dispatcher (the next element
    ///   in the dispatch pipeline) and returns a new dispatcher (the middleware you want to install in the pipeline).
    /// - Returns: This object adapter.
    @discardableResult
    func use(_ middlewareFactory: @escaping (_ next: Dispatcher) -> Dispatcher) -> Self

    /// Adds a servant to this object adapter's Active Servant Map (ASM). The ASM is a map {identity, facet} -> servant.
    /// Adding a servant with an identity that is in the map already throws ``AlreadyRegisteredException``.
    ///
    /// - Parameters:
    ///   - servant: The servant to add.
    ///   - id: The identity of the Ice object that is implemented by the servant.
    /// - Returns: A proxy for `id`, created by this object adapter.
    /// - Throws: `AlreadyRegisteredException` when a servant with the same identity is already registered.
    @discardableResult
    func add(servant: Dispatcher, id: Identity) throws -> ObjectPrx

    /// Adds a servant to this object adapter's Active Servant Map (ASM), while specifying a facet.
    /// The ASM is a map {identity, facet} -> servant.
    /// Adding a servant with an identity and facet that is in the map already throws ``AlreadyRegisteredException``.
    ///
    /// - Parameters:
    ///   - servant: The servant to add.
    ///   - id: The identity of the Ice object that is implemented by the servant.
    ///   - facet: The facet of the Ice object that is implemented by the servant.
    /// - Returns: A proxy for `id` and `facet`, created by this object adapter.
    /// - Throws: `AlreadyRegisteredException` when a servant with the same identity and facet is already registered.
    @discardableResult
    func addFacet(servant: Dispatcher, id: Identity, facet: String) throws -> ObjectPrx

    /// Adds a servant to this object adapter's Active Servant Map (ASM), using an automatically generated UUID as its
    /// identity.
    ///
    /// - Parameter servant: The servant to add.
    /// - Returns: A proxy with the generated UUID identity created by this object adapter.
    @discardableResult
    func addWithUUID(_ servant: Dispatcher) throws -> ObjectPrx

    /// Adds a servant to this object adapter's Active Servant Map (ASM), using an automatically generated UUID as its
    /// identity.  Also specifies a facet.
    ///
    /// - Parameters:
    ///   - servant: The servant to add.
    ///   - facet: The facet of the Ice object that is implemented by the servant.
    /// - Returns: A proxy with the generated UUID identity and specified facet created by this object adapter.
    @discardableResult
    func addFacetWithUUID(servant: Dispatcher, facet: String) throws -> ObjectPrx

    /// Adds a default servant to handle requests for a specific category. When an object adapter dispatches an
    /// incoming request, it tries to find a servant for the identity and facet carried by the request in the
    /// following order:
    /// - The object adapter tries to find a servant for the identity and facet in the Active Servant Map.
    /// - If this fails, the object adapter tries to find a default servant for the category component of the identity.
    /// - If this fails, the object adapter tries to find a default servant for the empty category, regardless of
    ///   the category contained in the identity.
    /// - If this fails, the object adapter tries to find a servant locator for the category component of the identity.
    ///   If there is no such servant locator, the object adapter tries to find a servant locator for the empty category.
    ///   - If a servant locator is found, the object adapter tries to find a servant using this servant locator.
    /// - If all the previous steps fail, the object adapter gives up and the caller receives an ``ObjectNotExistException``
    ///   or a ``FacetNotExistException``.
    ///
    /// - Parameters:
    ///   - servant: The default servant to add.
    ///   - category: The category for which the default servant is registered. The empty category means it handles all
    ///     categories.
    /// - Throws: `AlreadyRegisteredException` when a default servant with the same category is already registered.
    func addDefaultServant(servant: Dispatcher, category: String) throws

    /// Removes a servant from the object adapter's Active Servant Map.
    ///
    /// - Parameter id: The identity of the Ice object that is implemented by the servant.
    /// - Returns: The removed servant.
    /// - Throws: `NotRegisteredException` when no servant with the given identity is registered.
    @discardableResult
    func remove(_ id: Identity) throws -> Dispatcher

    /// Removes a servant from the object adapter's Active Servant Map, while specifying a facet.
    ///
    /// - Parameters:
    ///   - id: The identity of the Ice object that is implemented by the servant.
    ///   - facet: The facet. An empty facet means the default facet.
    /// - Returns: The removed servant.
    /// - Throws: `NotRegisteredException` when no servant with the given identity and facet is registered.
    @discardableResult
    func removeFacet(id: Identity, facet: String) throws -> Dispatcher

    /// Removes all facets with the given identity from the Active Servant Map. This method completely removes the
    /// Ice object, including its default facet.
    ///
    /// - Parameter id: The identity of the Ice object to be removed.
    /// - Returns: A collection containing all the facet names and servants of the removed Ice object.
    /// - Throws: `NotRegisteredException` when no servant with the given identity is registered.
    @discardableResult
    func removeAllFacets(_ id: Identity) throws -> FacetMap

    /// Removes the default servant for a specific category.
    ///
    /// - Parameter category: The category of the default servant to remove.
    /// - Returns: The default servant.
    /// - Throws: `NotRegisteredException` when no default servant is registered for the given category.
    @discardableResult
    func removeDefaultServant(_ category: String) throws -> Dispatcher

    /// Looks up a servant by identity in the Active Servant Map or among the default servants.
    /// This method does not attempt to locate a servant using servant locators.
    ///
    /// - Parameter id: The identity of an Ice object.
    /// - Returns: The servant that implements the Ice object with the given identity, or nil if no such
    ///   servant has been found.
    func find(_ id: Identity) -> Dispatcher?

    /// Looks up a servant by identity and facet in the Active Servant Map or among the default servants.
    /// This method does not attempt to locate a servant using servant locators.
    ///
    /// - Parameters:
    ///   - id: The identity of an Ice object.
    ///   - facet: The facet of an Ice object. An empty facet means the default facet.
    /// - Returns: The servant that implements the Ice object with the given identity and facet, or nil
    ///   if no such servant has been found.
    func findFacet(id: Identity, facet: String) -> Dispatcher?

    /// Finds all facets for a given identity in the Active Servant Map.
    ///
    /// - Parameter id: The identity.
    /// - Returns: A collection containing all the facet names and servants that have been found. Can be empty.
    func findAllFacets(_ id: Identity) -> FacetMap

    /// Looks up a servant in this object adapter's Active Servant Map or among the default servants, given a proxy.
    /// This operation does not attempt to locate a servant using servant locators.
    ///
    /// - Parameter proxy: The proxy that provides the identity and facet to search.
    /// - Returns: The servant that matches the identity and facet carried by `proxy`,
    ///   or nil if no such servant has been found.
    func findByProxy(_ proxy: ObjectPrx) -> Dispatcher?

    /// Adds a ``ServantLocator``` to this object adapter for a specific category.
    ///
    /// - Parameters:
    ///   - locator: The servant locator to add.
    ///   - category: The category. The empty category means `locator` handles all categories.
    /// - Throws: `AlreadyRegisteredException` when a servant locator with the same category is already registered.
    func addServantLocator(locator: ServantLocator, category: String) throws

    /// Removes a ``ServantLocator`` from this object adapter.
    ///
    /// - Parameter category: The category.
    /// - Returns: The servant locator.
    /// - Throws: `NotRegisteredException` when no servant locator with the given category is registered.
    @discardableResult
    func removeServantLocator(_ category: String) throws -> ServantLocator

    /// Finds a ``ServantLocator`` registered with this object adapter.
    ///
    /// - Parameter category: The category.
    /// - Returns: The servant locator, or `nil` if not found.
    func findServantLocator(_ category: String) -> ServantLocator?

    /// Finds the default servant for a specific category.
    ///
    /// - Parameter category: The category.
    /// - Returns: The default servant, or `nil` if not found.
    func findDefaultServant(_ category: String) -> Dispatcher?

    /// Creates a proxy from an Ice identity. If this object adapter is configured with an adapter ID, the proxy is an
    /// indirect proxy that refers to this adapter ID. If a replica group ID is also defined, the proxy is an indirect
    /// proxy that refers to this replica group ID. Otherwise, the proxy is a direct proxy containing this object
    /// adapter's published endpoints.
    ///
    /// - Parameter id: An Ice identity.
    /// - Returns: A proxy with the given identity.
    func createProxy(_ id: Identity) throws -> ObjectPrx

    /// Creates a direct proxy from an Ice identity.
    ///
    /// - Parameter id: An Ice identity.
    /// - Returns: A proxy with the given identity and this published endpoints of this object adapter.
    func createDirectProxy(_ id: Identity) throws -> ObjectPrx

    /// Creates an indirect proxy for an Ice identity.
    ///
    /// - Parameter id: An Ice identity.
    /// - Returns: An indirect proxy with the given identity. If this object adapter is not configured with an adapter
    ///   ID or a replica group ID, the new proxy is a well-known proxy (i.e., an identity-only proxy).
    func createIndirectProxy(_ id: Identity) throws -> ObjectPrx

    /// Sets an Ice locator on this object adapter.
    ///
    /// - Parameter loc: The locator used by this object adapter.
    func setLocator(_ loc: LocatorPrx?) throws

    /// Gets the Ice locator used by this object adapter.
    ///
    /// - Returns: The locator used by this object adapter, or nil if no locator is used by this object adapter.
    func getLocator() -> LocatorPrx?

    /// Gets the set of endpoints configured on this object adapter.
    /// This method remains usable after the object adapter has been deactivated.
    ///
    /// - Returns: The set of endpoints.
    func getEndpoints() -> EndpointSeq

    /// Gets the set of endpoints that proxies created by this object adapter will contain.
    /// This method remains usable after the object adapter has been deactivated.
    ///
    /// - Returns: The set of published endpoints.
    func getPublishedEndpoints() -> EndpointSeq

    /// Sets the endpoints that proxies created by this object adapter will contain.
    ///
    /// - Parameter newEndpoints: The new set of endpoints that the object adapter will embed in proxies.
    func setPublishedEndpoints(_ newEndpoints: EndpointSeq) throws
}
