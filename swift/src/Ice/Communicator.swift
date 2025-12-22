// Copyright (c) ZeroC, Inc.

import Foundation

/// Communicator is the central object in Ice. Its responsibilities include:
/// - creating and managing outgoing connections
/// - executing callbacks in its client thread pool
/// - creating and destroying object adapters
/// - loading plug-ins
/// - managing properties (configuration), retries, logging, instrumentation, and more.
/// You create a communicator with `Ice.initialize`, and it's usually the first object you create when programming
/// with Ice. You can create multiple communicators in a single program, but this is not common.
public protocol Communicator: AnyObject, Sendable {
    /// Destroys this communicator. This method calls ``shutdown()`` implicitly. Calling `destroy` destroys all
    /// object adapters and closes all outgoing connections. This method waits for all outstanding dispatches to
    /// complete before returning. This includes "bidirectional dispatches" that execute on outgoing connections.
    func destroy()

    /// Shuts down this communicator. This method calls ``ObjectAdapter/deactivate()`` on all object adapters created
    /// by this communicator. Shutting down a communicator has no effect on outgoing connections.
    func shutdown()

    /// Waits for shutdown to complete. This method calls ``ObjectAdapter/waitForDeactivate()`` on all object adapters
    /// created by this communicator. In a client application that does not accept incoming connections, this
    /// method returns as soon as another thread calls ``shutdown()`` or ``destroy()`` on this communicator.
    func waitForShutdown()

    /// Waits asynchronously until this communicator is shut down.
    /// If the communicator is already shut down, this call returns immediately.
    func shutdownCompleted() async

    /// Checks whether or not ``shutdown()`` was called on this communicator.
    ///
    /// - Returns: `true` if ``shutdown()`` was called on this communicator, `false` otherwise.
    func isShutdown() -> Bool

    /// Converts a stringified proxy into a proxy.
    ///
    /// - Parameter str: The stringified proxy to convert into a proxy.
    /// - Returns: The proxy, or `nil` if `str` is an empty string.
    /// - Throws: `ParseException` when `str` is not a valid proxy string.
    func stringToProxy(_ str: String) throws -> ObjectPrx?

    /// Converts a proxy into a string.
    ///
    /// - Parameter obj: The proxy to convert into a stringified proxy.
    /// - Returns: The stringified proxy, or an empty string if `obj` is `nil`.
    func proxyToString(_ obj: ObjectPrx?) -> String

    /// Converts a set of proxy properties into a proxy. The "base" name supplied in the `property` argument refers
    /// to a property containing a stringified proxy, such as `MyProxy=id:tcp -h localhost -p 10000`.
    /// Additional properties configure local settings for the proxy.
    ///
    /// - Parameter property: The base property name.
    /// - Returns: The proxy, or `nil` if the property is not set.
    func propertyToProxy(_ property: String) throws -> ObjectPrx?

    /// Converts a proxy into a set of proxy properties.
    ///
    /// - Parameters:
    ///   - proxy: The proxy.
    ///   - property: The base property name.
    /// - Returns: The property set.
    func proxyToProperty(proxy: ObjectPrx, property: String) -> PropertyDict

    /// Converts an identity into a string.
    ///
    /// - Parameter ident: The identity to convert into a string.
    /// - Returns: The "stringified" identity.
    func identityToString(_ ident: Identity) -> String

    /// Creates a new object adapter. The endpoints for the object adapter are taken from the property
    /// `name.Endpoints`.
    /// It is legal to create an object adapter with the empty string as its name. Such an object adapter is
    /// accessible via bidirectional connections or by collocated invocations.
    ///
    /// - Parameter name: The object adapter name.
    /// - Returns: The new object adapter.
    func createObjectAdapter(_ name: String) throws -> ObjectAdapter

    /// Creates a new object adapter with endpoints. This method sets the property `name.Endpoints`, and then
    /// calls ``createObjectAdapter(_:)``. It is provided as a convenience method. Calling this method with an empty
    /// name will result in a UUID being generated for the name.
    ///
    /// - Parameters:
    ///   - name: The object adapter name.
    ///   - endpoints: The endpoints of the object adapter.
    /// - Returns: The new object adapter.
    func createObjectAdapterWithEndpoints(name: String, endpoints: String) throws -> ObjectAdapter

    /// Creates a new object adapter with a router. This method creates a routed object adapter. Calling this
    /// method with an empty name will result in a UUID being generated for the name.
    ///
    /// - Parameters:
    ///   - name: The object adapter name.
    ///   - rtr: The router.
    /// - Returns: The new object adapter.
    func createObjectAdapterWithRouter(name: String, rtr: RouterPrx) throws -> ObjectAdapter

    /// Gets the object adapter that is associated by default with new outgoing connections created by this
    /// communicator. This method returns `nil` unless you set a non-`nil` default object adapter using
    /// ``setDefaultObjectAdapter(_:)``.
    ///
    /// - Returns: The object adapter associated by default with new outgoing connections.
    func getDefaultObjectAdapter() -> ObjectAdapter?

    /// Sets the object adapter that will be associated with new outgoing connections created by this communicator.
    /// This method has no effect on existing outgoing connections, or on incoming connections.
    ///
    /// - Parameter adapter: The object adapter to associate with new outgoing connections.
    func setDefaultObjectAdapter(_ adapter: ObjectAdapter?)

    /// Gets the implicit context associated with this communicator.
    ///
    /// - Returns: The implicit context associated with this communicator; returns `nil` when the
    ///   property `Ice.ImplicitContext` is not set or is set to `None`.
    func getImplicitContext() -> ImplicitContext?

    /// Gets the properties of this communicator.
    ///
    /// - Returns: This communicator's properties.
    func getProperties() -> Properties

    /// Gets the logger of this communicator.
    ///
    /// - Returns: This communicator's logger.
    func getLogger() -> Logger

    /// Gets the default router of this communicator.
    ///
    /// - Returns: The default router of this communicator.
    func getDefaultRouter() -> RouterPrx?

    /// Sets the default router of this communicator. All newly created proxies will use this default router.
    /// This method has no effect on existing proxies.
    ///
    /// - Parameter rtr: The new default router. Use `nil` to remove the default router.
    func setDefaultRouter(_ rtr: RouterPrx?)

    /// Gets the default locator of this communicator.
    ///
    /// - Returns: The default locator of this communicator.
    func getDefaultLocator() -> LocatorPrx?

    /// Sets the default locator of this communicator. All newly created proxies will use this default locator.
    /// This method has no effect on existing proxies or object adapters.
    ///
    /// - Parameter loc: The new default locator. Use `nil` to remove the default locator.
    func setDefaultLocator(_ loc: LocatorPrx?)

    /// Flushes any pending batch requests of this communicator. This means all batch requests invoked on fixed
    /// proxies for all connections associated with the communicator. Errors that occur while flushing a connection
    /// are ignored.
    ///
    /// - Parameter compress: Specifies whether or not the queued batch requests should be compressed before being
    ///   sent over the wire.
    func flushBatchRequests(_ compress: CompressBatch) async throws

    /// Adds the Admin object with all its facets to the provided object adapter. If `Ice.Admin.ServerId`
    /// is set and the provided object adapter has a Locator, `createAdmin` registers the Admin's Process facet with
    /// the Locator's LocatorRegistry.
    ///
    /// - Parameters:
    ///   - adminAdapter: The object adapter used to host the Admin object; if it is `nil` and `Ice.Admin.Endpoints`
    ///     is set, this method uses the `Ice.Admin` object adapter, after creating and activating it.
    ///   - adminId: The identity of the Admin object.
    /// - Returns: A proxy to the main ("") facet of the Admin object.
    /// - Throws: `InitializationException` when `createAdmin` is called more than once.
    func createAdmin(adminAdapter: ObjectAdapter?, adminId: Identity) throws -> ObjectPrx

    /// Gets a proxy to the main facet of the Admin object. `getAdmin` also creates the Admin object and creates and
    /// activates the `Ice.Admin` object adapter to host this Admin object if `Ice.Admin.Endpoints` is set. The
    /// identity of the Admin object created by `getAdmin` is `{value of Ice.Admin.InstanceName}/admin`, or
    /// `{UUID}/admin` when `Ice.Admin.InstanceName` is not set. If `Ice.Admin.DelayCreation` is `0` or not set,
    /// `getAdmin` is called by the communicator initialization, after initialization of all plugins.
    ///
    /// - Returns: A proxy to the main ("") facet of the Admin object, or `nil` if no Admin object is configured.
    func getAdmin() throws -> ObjectPrx?

    /// Adds a new facet to the Admin object.
    ///
    /// - Parameters:
    ///   - servant: The servant that implements the new Admin facet.
    ///   - facet: The name of the new Admin facet.
    /// - Throws: `AlreadyRegisteredException` when a facet with the same name is already registered.
    func addAdminFacet(servant: Dispatcher, facet: String) throws

    /// Removes a facet from the Admin object.
    ///
    /// - Parameter facet: The name of the Admin facet.
    /// - Returns: The servant associated with this Admin facet.
    /// - Throws: `NotRegisteredException` when no facet with the given name is registered.
    @discardableResult
    func removeAdminFacet(_ facet: String) throws -> Dispatcher

    /// Returns a facet of the Admin object.
    ///
    /// - Parameter facet: The name of the Admin facet.
    /// - Returns: The servant associated with this Admin facet, or nil if no facet is registered with the
    ///   given name.
    func findAdminFacet(_ facet: String) -> Dispatcher?

    /// Returns a map of all facets of the Admin object.
    ///
    /// - Returns: A collection containing all the facet names and servants of the Admin object.
    func findAllAdminFacets() -> FacetMap

    /// Makes a new proxy. This is an internal operation used by the generated code.
    ///
    /// - Parameter proxyString: The stringified proxy.
    /// - Returns: The new proxy.
    func makeProxyImpl<ProxyImpl>(_ proxyString: String) throws -> ProxyImpl
    where ProxyImpl: ObjectPrxI
}
