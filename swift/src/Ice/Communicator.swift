// Copyright (c) ZeroC, Inc.

import Foundation

/// The central object in Ice. One or more communicators can be instantiated for an Ice application. Communicator
/// instantiation is language-specific, and not specified in Slice code.
public protocol Communicator: AnyObject, Sendable {
    /// Destroys this communicator. This method calls `shutdown` implicitly. Calling `destroy` destroys all
    /// object adapters, and closes all outgoing connections. `destroy` waits for all outstanding dispatches to
    /// complete before returning. This includes "bidirectional dispatches" that execute on outgoing connections.
    func destroy()

    /// Shuts down this communicator. This function calls `ObjectAdapter.deactivate` on all object adapters created
    /// by this communicator. Shutting down a communicator has no effect on outgoing connections.
    func shutdown()

    /// Waits for shutdown to complete. This method calls `ObjectAdapter.waitForDeactivate` on all object adapters
    /// created by this communicator. In a client application that does not accept incoming connections, this
    /// method returns as soon as another thread calls `shutdown` or `destroy` on this communicator.
    func waitForShutdown()

    /// Waits asynchronously until the communicator is shut down. This is the asynchronous equivalent of
    /// `waitForShutdown()`.
    func shutdownCompleted() async

    /// Checks whether or not `shutdown` was called on this communicator.
    ///
    /// - Returns: True if shutdown was called on this communicator; false otherwise.
    func isShutdown() -> Bool

    /// Convert a stringified proxy into a proxy.
    /// For example, MyCategory/MyObject:tcp -h some_host -p 10000 creates a proxy that refers to the Ice
    /// object having an identity with a name "MyObject" and a category "MyCategory", with the server running on host
    /// "some_host", port 10000. If the stringified proxy does not parse correctly, the operation throws ParseException.
    /// Refer to the Ice manual for a detailed description of the syntax supported by stringified proxies.
    ///
    /// - Parameter str: The stringified proxy to convert into a proxy.
    /// - Returns: The proxy, or nil if str is an empty string.
    func stringToProxy(_ str: String) throws -> ObjectPrx?

    /// Convert a proxy into a string.
    ///
    /// - Parameter obj: The proxy to convert into a stringified proxy.
    /// - Returns: The stringified proxy, or an empty string if obj is nil.
    func proxyToString(_ obj: ObjectPrx?) -> String

    /// Convert a set of proxy properties into a proxy. The "base" name supplied in the property argument
    /// refers to a property containing a stringified proxy, such as MyProxy=id:tcp -h localhost -p 10000.
    /// Additional properties configure local settings for the proxy.
    ///
    /// - Parameter property: The base property name.
    /// - Returns: The proxy, or nil if the property is not set.
    func propertyToProxy(_ property: String) throws -> ObjectPrx?

    /// Convert a proxy to a set of proxy properties.
    ///
    /// - Parameters:
    ///   - proxy: The proxy.
    ///   - property: The base property name.
    /// - Returns: The property set.
    func proxyToProperty(proxy: ObjectPrx, property: String) -> PropertyDict

    /// Convert an identity into a string.
    ///
    /// - Parameter ident: The identity to convert into a string.
    /// - Returns: The "stringified" identity.
    func identityToString(_ ident: Identity) -> String

    /// Create a new object adapter. The endpoints for the object adapter are taken from the property
    /// name.Endpoints.
    /// It is legal to create an object adapter with the empty string as its name. Such an object adapter is accessible
    /// via bidirectional connections or by collocated invocations that originate from the same communicator as is used
    /// by the adapter. Attempts to create a named object adapter for which no configuration can be found raise
    /// InitializationException.
    ///
    /// - Parameter name: The object adapter name.
    /// - Returns: The new object adapter.
    func createObjectAdapter(_ name: String) throws -> ObjectAdapter

    /// Create a new object adapter with endpoints. This operation sets the property
    /// name.Endpoints, and then calls createObjectAdapter. It is provided as a
    /// convenience function. Calling this operation with an empty name will result in a UUID being generated for the
    /// name.
    ///
    /// - Parameters:
    ///   - name: The object adapter name.
    ///   - endpoints: The endpoints for the object adapter.
    /// - Returns: The new object adapter.
    func createObjectAdapterWithEndpoints(name: String, endpoints: String) throws
        -> ObjectAdapter

    /// Create a new object adapter with a router. This operation creates a routed object adapter.
    /// Calling this operation with an empty name will result in a UUID being generated for the name.
    ///
    /// - Parameters:
    ///   - name: The object adapter name.
    ///   - rtr: The router.
    /// - Returns: The new object adapter.
    func createObjectAdapterWithRouter(name: String, rtr: RouterPrx) throws -> ObjectAdapter

    /// Gets the object adapter that is associated by default with new outgoing connections created by this
    /// communicator. This function returns nil unless you set a non-nil default object adapter using
    /// setDefaultObjectAdapter.
    ///
    /// - Returns: The object adapter associated by default with new outgoing connections.
    func getDefaultObjectAdapter() -> ObjectAdapter?

    /// Sets the object adapter that will be associated with new outgoing connections created by this communicator. This
    /// function has no effect on existing outgoing connections, or on incoming connections.
    ///
    /// - Parameter adapter: The object adapter to associate with new outgoing connections.
    func setDefaultObjectAdapter(_ adapter: ObjectAdapter?)

    /// Get the implicit context associated with this communicator.
    ///
    /// - Returns: The implicit context associated with this communicator; returns nil when the
    ///            property Ice.ImplicitContext is not set or is set to None.
    func getImplicitContext() -> ImplicitContext?

    /// Get the properties for this communicator.
    ///
    /// - Returns: This communicator's properties.
    func getProperties() -> Properties

    /// Get the logger for this communicator.
    ///
    /// - Returns: This communicator's logger.
    func getLogger() -> Logger

    /// Get the default router for this communicator.
    ///
    /// - Returns: The default router for this communicator.
    func getDefaultRouter() -> RouterPrx?

    /// Set a default router for this communicator. All newly created proxies will use this default router. To disable
    /// the default router, nil can be used. Note that this operation has no effect on existing proxies.
    /// You can also set a router for an individual proxy by calling the operation ice_router on the
    /// proxy.
    ///
    /// - Parameter rtr: The default router to use for this communicator.
    func setDefaultRouter(_ rtr: RouterPrx?)

    /// Get the default locator for this communicator.
    ///
    /// - Returns: The default locator for this communicator.
    func getDefaultLocator() -> LocatorPrx?

    /// Set a default Ice locator for this communicator. All newly created proxy and object adapters will use this
    /// default locator. To disable the default locator, nil can be used. Note that this operation has no effect on
    /// existing proxies or object adapters.
    /// You can also set a locator for an individual proxy by calling the operation ice_locator on the
    /// proxy, or for an object adapter by calling ObjectAdapter.setLocator on the object adapter.
    ///
    /// - Parameter loc: The default locator to use for this communicator.
    func setDefaultLocator(_ loc: LocatorPrx?)

    /// Flush any pending batch requests for this communicator. This means all batch requests invoked on fixed proxies
    /// for all connections associated with the communicator. Any errors that occur while flushing a connection are
    /// ignored.
    ///
    /// - Parameter compress: Specifies whether or not the queued batch requests should be compressed before
    ///                       being sent over the wire.
    func flushBatchRequests(_ compress: CompressBatch) async throws

    /// Add the Admin object with all its facets to the provided object adapter. If Ice.Admin.ServerId is
    /// set and the provided object adapter has a Locator, createAdmin registers the Admin's Process facet with
    /// the Locator's LocatorRegistry. createAdmin must only be called once; subsequent calls raise
    /// InitializationException.
    ///
    /// - Parameter adminAdapter: The object adapter used to host the Admin object; if nil and
    ///                          Ice.Admin.Endpoints is set, create, activate and use the Ice.Admin object adapter.
    /// - Parameter adminId: The identity of the Admin object.
    /// - Returns: A proxy to the main ("") facet of the Admin object. Never returns a nil proxy.
    func createAdmin(adminAdapter: ObjectAdapter?, adminId: Identity) throws -> ObjectPrx

    /// Get a proxy to the main facet of the Admin object. getAdmin also creates the Admin object and creates and
    /// activates the Ice.Admin object adapter to host this Admin object if Ice.Admin.Endpoints is set. The identity of
    /// the Admin object created by getAdmin is {value of Ice.Admin.InstanceName}/admin, or {UUID}/admin when
    /// Ice.Admin.InstanceName is not set. If Ice.Admin.DelayCreation is 0 or not set, getAdmin is called
    /// by the communicator initialization, after initialization of all plugins.
    ///
    /// - Returns: A proxy to the main ("") facet of the Admin object, or a nil proxy if no Admin
    ///            object is configured.
    func getAdmin() throws -> ObjectPrx?

    /// Add a new facet to the Admin object. Adding a servant with a facet that is already registered throws
    /// AlreadyRegisteredException.
    ///
    /// - Parameters:
    ///   - servant: The servant that implements the new Admin facet.
    ///   - facet: The name of the new Admin facet.
    func addAdminFacet(servant: Dispatcher, facet: String) throws

    /// Remove the following facet to the Admin object. Removing a facet that was not previously registered throws
    /// NotRegisteredException.
    ///
    /// - Parameter facet: The name of the Admin facet.
    /// - Returns: The servant associated with this Admin facet.
    @discardableResult
    func removeAdminFacet(_ facet: String) throws -> Dispatcher

    /// Returns a facet of the Admin object.
    ///
    /// - Parameter facet: The name of the Admin facet.
    /// - Returns: The servant associated with this Admin facet, or nil if no facet is registered with the
    ///            given name.
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
