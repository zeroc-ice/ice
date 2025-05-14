// Copyright (c) ZeroC, Inc.

import Foundation

/// The object adapter provides an up-call interface from the Ice run time to the implementation of Ice objects. The
/// object adapter is responsible for receiving requests from endpoints, and for mapping between servants, identities,
/// and proxies.
public protocol ObjectAdapter: AnyObject {

    /// Get the dispatch pipeline of this object adapter.
    var dispatchPipeline: Dispatcher { get }

    /// Get the name of this object adapter.
    ///
    /// - returns: `String` - This object adapter's name.
    func getName() -> String

    /// Get the communicator this object adapter belongs to.
    ///
    /// - returns: `Communicator` - This object adapter's communicator.
    func getCommunicator() -> Communicator

    /// Activate all endpoints that belong to this object adapter. After activation, the object adapter can dispatch
    /// requests received through its endpoints.
    func activate() throws

    /// Temporarily hold receiving and dispatching requests. The object adapter can be reactivated with the
    /// activate operation.  Holding is not immediate, i.e., after hold returns, the
    /// object adapter might still be active for some time. You can use waitForHold to wait until holding is
    /// complete.
    func hold()

    /// Wait until the object adapter holds requests. Calling hold initiates holding of requests, and
    /// waitForHold only returns when holding of requests has been completed.
    func waitForHold()

    /// Deactivates this object adapter: stop accepting new connections from clients and close gracefully all incoming
    /// connections created by this object adapter once all outstanding dispatches have completed. If this object
    /// adapter is indirect, this function also unregisters the object adapter from the Locator.
    /// This function does not cancel outstanding dispatches--it lets them execute until completion. A new incoming
    /// request on an existing connection will be accepted and can delay the closure of the connection.
    /// A deactivated object adapter cannot be reactivated again; it can only be destroyed.
    func deactivate()

    /// Wait until deactivate is called on this object adapter and all connections accepted by this object adapter are
    /// closed. A connection is closed only after all outstanding dispatches on this connection have completed.
    func waitForDeactivate()

    /// Checks if this object adapter has been deactivated.
    ///
    /// - returns: `Bool` - Whether adapter has been deactivated.
    func isDeactivated() -> Bool

    /// Destroys this object adapter and cleans up all resources held by this object adapter.
    /// Once this function has returned, it is possible to create another object adapter with the same name.
    func destroy()

    /// Add a middleware to the dispatch pipeline of this object adapter.
    ///
    /// - Parameter middlewareFactory: The middleware factory that creates the new middleware when this object adapter
    /// creates its dispatch pipeline. A middleware factory is a function that takes a dispatcher (the next element in
    /// the dispatch pipeline) and returns a new dispatcher (the middleware you want to install in the pipeline).
    /// - Returns: This object adapter.
    ///
    /// - Note: All middleware must be installed before the first dispatch.
    /// - Note: The middleware are executed in the order they are installed.
    @discardableResult
    func use(_ middlewareFactory: @escaping (_ next: Dispatcher) -> Dispatcher) -> Self

    /// Add a servant to this object adapter's Active Servant Map. Note that one servant can implement several Ice
    /// objects by registering the servant with multiple identities. Adding a servant with an identity that is in the
    /// map already throws AlreadyRegisteredException.
    ///
    /// - parameter servant: `Dispatcher` The servant to add.
    ///
    /// - parameter id: `Identity` The identity of the Ice object that is implemented by the servant.
    ///
    /// - returns: `ObjectPrx` - A proxy that matches the given identity and this object adapter.
    @discardableResult
    func add(servant: Dispatcher, id: Identity) throws -> ObjectPrx

    /// Like add, but with a facet. Calling add(servant, id) is equivalent to calling
    /// addFacet with an empty facet.
    ///
    /// - parameter servant: `Dispatcher` The servant to add.
    ///
    /// - parameter id: `Identity` The identity of the Ice object that is implemented by the servant.
    ///
    /// - parameter facet: `String` The facet. An empty facet means the default facet.
    ///
    /// - returns: `ObjectPrx` - A proxy that matches the given identity, facet, and this object adapter.
    @discardableResult
    func addFacet(servant: Dispatcher, id: Identity, facet: String) throws -> ObjectPrx

    /// Add a servant to this object adapter's Active Servant Map, using an automatically generated UUID as its
    /// identity. Note that the generated UUID identity can be accessed using the proxy's ice_getIdentity
    /// operation.
    ///
    /// - parameter servant: `Dispatcher` The servant to add.
    ///
    /// - returns: `ObjectPrx` - A proxy that matches the generated UUID identity and this object adapter.
    @discardableResult
    func addWithUUID(_ servant: Dispatcher) throws -> ObjectPrx

    /// Like addWithUUID, but with a facet. Calling addWithUUID(servant) is equivalent to calling
    /// addFacetWithUUID with an empty facet.
    ///
    /// - parameter servant: `Dispatcher` The servant to add.
    ///
    /// - parameter facet: `String` The facet. An empty facet means the default facet.
    ///
    /// - returns: `ObjectPrx` - A proxy that matches the generated UUID identity, facet, and this object adapter.
    @discardableResult
    func addFacetWithUUID(servant: Dispatcher, facet: String) throws -> ObjectPrx

    /// Add a default servant to handle requests for a specific category. Adding a default servant for a category for
    /// which a default servant is already registered throws AlreadyRegisteredException. To dispatch operation
    /// calls on servants, the object adapter tries to find a servant for a given Ice object identity and facet in the
    /// following order:
    ///
    /// The object adapter tries to find a servant for the identity and facet in the Active Servant Map.
    /// If no servant has been found in the Active Servant Map, the object adapter tries to find a default servant
    /// for the category component of the identity.
    /// If no servant has been found by any of the preceding steps, the object adapter tries to find a default
    /// servant for an empty category, regardless of the category contained in the identity.
    /// If no servant has been found by any of the preceding steps, the object adapter gives up and the caller
    /// receives ObjectNotExistException or FacetNotExistException.
    ///
    /// - parameter servant: `Dispatcher` The default servant.
    ///
    /// - parameter category: `String` The category for which the default servant is registered. An empty
    /// category means it will handle all categories.
    func addDefaultServant(servant: Dispatcher, category: String) throws

    /// Remove a servant (that is, the default facet) from the object adapter's Active Servant Map.
    ///
    /// - parameter id: `Identity` The identity of the Ice object that is implemented by the servant. If the servant
    /// implements multiple Ice objects, remove has to be called for all those Ice objects. Removing an identity
    /// that is not in the map throws NotRegisteredException.
    ///
    /// - returns: `Dispatcher` - The removed servant.
    @discardableResult
    func remove(_ id: Identity) throws -> Dispatcher

    /// Like remove, but with a facet. Calling remove(id) is equivalent to calling
    /// removeFacet with an empty facet.
    ///
    /// - parameter id: `Identity` The identity of the Ice object that is implemented by the servant.
    ///
    /// - parameter facet: `String` The facet. An empty facet means the default facet.
    ///
    /// - returns: `Dispatcher` - The removed servant.
    @discardableResult
    func removeFacet(id: Identity, facet: String) throws -> Dispatcher

    /// Remove all facets with the given identity from the Active Servant Map. The operation completely removes the Ice
    /// object, including its default facet. Removing an identity that is not in the map throws
    /// NotRegisteredException.
    ///
    /// - parameter id: `Identity` The identity of the Ice object to be removed.
    ///
    /// - returns: `FacetMap` - A collection containing all the facet names and servants of the removed Ice object.
    @discardableResult
    func removeAllFacets(_ id: Identity) throws -> FacetMap

    /// Remove the default servant for a specific category. Attempting to remove a default servant for a category that
    /// is not registered throws NotRegisteredException.
    ///
    /// - parameter category: `String` The category of the default servant to remove.
    ///
    /// - returns: `Dispatcher` - The default servant.
    @discardableResult
    func removeDefaultServant(_ category: String) throws -> Dispatcher

    /// Look up a servant in this object adapter's Active Servant Map by the identity of the Ice object it implements.
    /// This operation only tries to look up a servant in the Active Servant Map. It does not attempt
    /// to find a servant by using any installed ServantLocator.
    ///
    /// - parameter id: `Identity` The identity of the Ice object for which the servant should be returned.
    ///
    /// - returns: `Dispatcher?` - The servant that implements the Ice object with the given identity, or null if no such
    /// servant has been found.
    func find(_ id: Identity) -> Dispatcher?

    /// Like find, but with a facet. Calling find(id) is equivalent to calling findFacet
    /// with an empty facet.
    ///
    /// - parameter id: `Identity` The identity of the Ice object for which the servant should be returned.
    ///
    /// - parameter facet: `String` The facet. An empty facet means the default facet.
    ///
    /// - returns: `Dispatcher?` - The servant that implements the Ice object with the given identity and facet, or null if
    /// no such servant has been found.
    func findFacet(id: Identity, facet: String) -> Dispatcher?

    /// Find all facets with the given identity in the Active Servant Map.
    ///
    /// - parameter id: `Identity` The identity of the Ice object for which the facets should be returned.
    ///
    /// - returns: `FacetMap` - A collection containing all the facet names and servants that have been found, or an
    /// empty map if there is no facet for the given identity.
    func findAllFacets(_ id: Identity) -> FacetMap

    /// Look up a servant in this object adapter's Active Servant Map, given a proxy.
    /// This operation only tries to lookup a servant in the Active Servant Map. It does not attempt to
    /// find a servant by using any installed ServantLocator.
    ///
    /// - parameter proxy: `ObjectPrx` The proxy for which the servant should be returned.
    ///
    /// - returns: `Dispatcher?` - The servant that matches the proxy, or null if no such servant has been found.
    func findByProxy(_ proxy: ObjectPrx) -> Dispatcher?

    /// Add a Servant Locator to this object adapter. Adding a servant locator for a category for which a servant
    /// locator is already registered throws AlreadyRegisteredException. To dispatch operation calls on
    /// servants, the object adapter tries to find a servant for a given Ice object identity and facet in the following
    /// order:
    ///
    /// The object adapter tries to find a servant for the identity and facet in the Active Servant Map.
    /// If no servant has been found in the Active Servant Map, the object adapter tries to find a servant locator
    /// for the category component of the identity. If a locator is found, the object adapter tries to find a servant
    /// using this locator.
    /// If no servant has been found by any of the preceding steps, the object adapter tries to find a locator for
    /// an empty category, regardless of the category contained in the identity. If a locator is found, the object
    /// adapter tries to find a servant using this locator.
    /// If no servant has been found by any of the preceding steps, the object adapter gives up and the caller
    /// receives ObjectNotExistException or FacetNotExistException.
    ///
    /// Only one locator for the empty category can be installed.
    ///
    /// - parameter locator: `ServantLocator` The locator to add.
    ///
    /// - parameter category: `String` The category for which the Servant Locator can locate servants, or an
    /// empty string if the Servant Locator does not belong to any specific category.
    func addServantLocator(locator: ServantLocator, category: String) throws

    /// Remove a Servant Locator from this object adapter.
    ///
    /// - parameter category: `String` The category for which the Servant Locator can locate servants, or an empty
    /// string if the Servant Locator does not belong to any specific category.
    ///
    /// - returns: `ServantLocator` - The Servant Locator, or throws NotRegisteredException if no Servant Locator was
    /// found for the given category.
    @discardableResult
    func removeServantLocator(_ category: String) throws -> ServantLocator

    /// Find a Servant Locator installed with this object adapter.
    ///
    /// - parameter category: `String` The category for which the Servant Locator can locate servants, or an empty
    /// string if the Servant Locator does not belong to any specific category.
    ///
    /// - returns: `ServantLocator?` - The Servant Locator, or null if no Servant Locator was found for the given
    /// category.
    func findServantLocator(_ category: String) -> ServantLocator?

    /// Find the default servant for a specific category.
    ///
    /// - parameter category: `String` The category of the default servant to find.
    ///
    /// - returns: `Dispatcher?` - The default servant or null if no default servant was registered for the category.
    func findDefaultServant(_ category: String) -> Dispatcher?

    /// Create a proxy for the object with the given identity. If this object adapter is configured with an adapter id,
    /// the return value is an indirect proxy that refers to the adapter id. If a replica group id is also defined, the
    /// return value is an indirect proxy that refers to the replica group id. Otherwise, if no adapter id is defined,
    /// the return value is a direct proxy containing this object adapter's published endpoints.
    ///
    /// - parameter id: `Identity` The object's identity.
    ///
    /// - returns: `ObjectPrx` - A proxy for the object with the given identity.
    func createProxy(_ id: Identity) throws -> ObjectPrx

    /// Create a direct proxy for the object with the given identity. The returned proxy contains this object adapter's
    /// published endpoints.
    ///
    /// - parameter id: `Identity` The object's identity.
    ///
    /// - returns: `ObjectPrx` - A proxy for the object with the given identity.
    func createDirectProxy(_ id: Identity) throws -> ObjectPrx

    /// Create an indirect proxy for the object with the given identity. If this object adapter is configured with an
    /// adapter id, the return value refers to the adapter id. Otherwise, the return value contains only the object
    /// identity.
    ///
    /// - parameter id: `Identity` The object's identity.
    ///
    /// - returns: `ObjectPrx` - A proxy for the object with the given identity.
    func createIndirectProxy(_ id: Identity) throws -> ObjectPrx

    /// Set an Ice locator for this object adapter. By doing so, the object adapter will register itself with the
    /// locator registry when it is activated for the first time. Furthermore, the proxies created by this object
    /// adapter will contain the adapter identifier instead of its endpoints. The adapter identifier must be configured
    /// using the AdapterId property.
    ///
    /// - parameter loc: `LocatorPrx?` The locator used by this object adapter.
    func setLocator(_ loc: LocatorPrx?) throws

    /// Get the Ice locator used by this object adapter.
    ///
    /// - returns: `LocatorPrx?` - The locator used by this object adapter, or null if no locator is used by this
    /// object adapter.
    func getLocator() -> LocatorPrx?

    /// Get the set of endpoints configured with this object adapter.
    ///
    /// - returns: `EndpointSeq` - The set of endpoints.
    func getEndpoints() -> EndpointSeq

    /// Get the set of endpoints that proxies created by this object adapter will contain.
    ///
    /// - returns: `EndpointSeq` - The set of published endpoints.
    func getPublishedEndpoints() -> EndpointSeq

    /// Set of the endpoints that proxies created by this object adapter will contain.
    ///
    /// - parameter newEndpoints: `EndpointSeq` The new set of endpoints that the object adapter will embed in proxies.
    func setPublishedEndpoints(_ newEndpoints: EndpointSeq) throws
}
