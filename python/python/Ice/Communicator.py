# Copyright (c) ZeroC, Inc. All rights reserved.

__name__ = "Ice"

class Communicator(object):
    """
    The central object in Ice. One or more communicators can be instantiated for an Ice application. Communicator
    instantiation is language-specific, and not specified in Slice code.
    """

    def __init__(self):
        if type(self) == Communicator:
            raise RuntimeError("Ice.Communicator is an abstract class")

    def destroy(self):
        """
        Destroy the communicator. This operation calls shutdown implicitly. Calling destroy cleans up
        memory, and shuts down this communicator's client functionality and destroys all object adapters. Subsequent
        calls to destroy are ignored.
        """
        raise NotImplementedError("method 'destroy' not implemented")

    def shutdown(self):
        """
        Shuts down this communicator's server functionality, which includes the deactivation of all object adapters.
        Attempts to use a deactivated object adapter raise ObjectAdapterDeactivatedException. Subsequent calls to
        shutdown are ignored.
        After shutdown returns, no new requests are processed. However, requests that have been started before shutdown
        was called might still be active. You can use waitForShutdown to wait for the completion of all
        requests.
        """
        raise NotImplementedError("method 'shutdown' not implemented")

    def waitForShutdown(self):
        """
        Wait until the application has called shutdown (or destroy). On the server side, this
        operation blocks the calling thread until all currently-executing operations have completed. On the client
        side, the operation simply blocks until another thread has called shutdown or destroy.
        A typical use of this operation is to call it from the main thread, which then waits until some other thread
        calls shutdown. After shut-down is complete, the main thread returns and can do some cleanup work
        before it finally calls destroy to shut down the client functionality, and then exits the application.
        """
        raise NotImplementedError("method 'waitForShutdown' not implemented")

    def isShutdown(self):
        """
            Check whether communicator has been shut down.
        Returns: True if the communicator has been shut down; false otherwise.
        """
        raise NotImplementedError("method 'isShutdown' not implemented")

    def stringToProxy(self, str):
        """
            Convert a stringified proxy into a proxy.
            For example, MyCategory/MyObject:tcp -h some_host -p 10000 creates a proxy that refers to the Ice
            object having an identity with a name "MyObject" and a category "MyCategory", with the server running on host
            "some_host", port 10000. If the stringified proxy does not parse correctly, the operation throws ParseException.
            Refer to the Ice manual for a detailed description of the syntax supported by stringified proxies.
        Arguments:
        str -- The stringified proxy to convert into a proxy.
        Returns: The proxy, or nil if str is an empty string.
        """
        raise NotImplementedError("method 'stringToProxy' not implemented")

    def proxyToString(self, obj):
        """
            Convert a proxy into a string.
        Arguments:
        obj -- The proxy to convert into a stringified proxy.
        Returns: The stringified proxy, or an empty string if obj is nil.
        """
        raise NotImplementedError("method 'proxyToString' not implemented")

    def propertyToProxy(self, property):
        """
            Convert a set of proxy properties into a proxy. The "base" name supplied in the property argument
            refers to a property containing a stringified proxy, such as MyProxy=id:tcp -h localhost -p 10000.
            Additional properties configure local settings for the proxy, such as MyProxy.PreferSecure=1. The
            "Properties" appendix in the Ice manual describes each of the supported proxy properties.
        Arguments:
        property -- The base property name.
        Returns: The proxy.
        """
        raise NotImplementedError("method 'propertyToProxy' not implemented")

    def proxyToProperty(self, proxy, property):
        """
            Convert a proxy to a set of proxy properties.
        Arguments:
        proxy -- The proxy.
        property -- The base property name.
        Returns: The property set.
        """
        raise NotImplementedError("method 'proxyToProperty' not implemented")

    def identityToString(self, ident):
        """
            Convert an identity into a string.
        Arguments:
        ident -- The identity to convert into a string.
        Returns: The "stringified" identity.
        """
        raise NotImplementedError("method 'identityToString' not implemented")

    def createObjectAdapter(self, name):
        """
            Create a new object adapter. The endpoints for the object adapter are taken from the property
            name.Endpoints.
            It is legal to create an object adapter with the empty string as its name. Such an object adapter is accessible
            via bidirectional connections or by collocated invocations that originate from the same communicator as is used
            by the adapter. Attempts to create a named object adapter for which no configuration can be found raise
            InitializationException.
        Arguments:
        name -- The object adapter name.
        Returns: The new object adapter.
        """
        raise NotImplementedError("method 'createObjectAdapter' not implemented")

    def createObjectAdapterWithEndpoints(self, name, endpoints):
        """
            Create a new object adapter with endpoints. This operation sets the property
            name.Endpoints, and then calls createObjectAdapter. It is provided as a
            convenience function. Calling this operation with an empty name will result in a UUID being generated for the
            name.
        Arguments:
        name -- The object adapter name.
        endpoints -- The endpoints for the object adapter.
        Returns: The new object adapter.
        """
        raise NotImplementedError(
            "method 'createObjectAdapterWithEndpoints' not implemented"
        )

    def createObjectAdapterWithRouter(self, name, rtr):
        """
            Create a new object adapter with a router. This operation creates a routed object adapter.
            Calling this operation with an empty name will result in a UUID being generated for the name.
        Arguments:
        name -- The object adapter name.
        rtr -- The router.
        Returns: The new object adapter.
        """
        raise NotImplementedError(
            "method 'createObjectAdapterWithRouter' not implemented"
        )

    def getImplicitContext(self):
        """
            Get the implicit context associated with this communicator.
        Returns: The implicit context associated with this communicator; returns null when the property Ice.ImplicitContext is not set or is set to None.
        """
        raise NotImplementedError("method 'getImplicitContext' not implemented")

    def getProperties(self):
        """
            Get the properties for this communicator.
        Returns: This communicator's properties.
        """
        raise NotImplementedError("method 'getProperties' not implemented")

    def getLogger(self):
        """
            Get the logger for this communicator.
        Returns: This communicator's logger.
        """
        raise NotImplementedError("method 'getLogger' not implemented")

    def getObserver(self):
        """
            Get the observer resolver object for this communicator.
        Returns: This communicator's observer resolver object.
        """
        raise NotImplementedError("method 'getObserver' not implemented")

    def getDefaultRouter(self):
        """
            Get the default router for this communicator.
        Returns: The default router for this communicator.
        """
        raise NotImplementedError("method 'getDefaultRouter' not implemented")

    def setDefaultRouter(self, rtr):
        """
            Set a default router for this communicator. All newly created proxies will use this default router. To disable
            the default router, null can be used. Note that this operation has no effect on existing proxies.
            You can also set a router for an individual proxy by calling the operation ice_router on the
            proxy.
        Arguments:
        rtr -- The default router to use for this communicator.
        """
        raise NotImplementedError("method 'setDefaultRouter' not implemented")

    def getDefaultLocator(self):
        """
            Get the default locator for this communicator.
        Returns: The default locator for this communicator.
        """
        raise NotImplementedError("method 'getDefaultLocator' not implemented")

    def setDefaultLocator(self, loc):
        """
            Set a default Ice locator for this communicator. All newly created proxy and object adapters will use this
            default locator. To disable the default locator, null can be used. Note that this operation has no effect on
            existing proxies or object adapters.
            You can also set a locator for an individual proxy by calling the operation ice_locator on the
            proxy, or for an object adapter by calling ObjectAdapter#setLocator on the object adapter.
        Arguments:
        loc -- The default locator to use for this communicator.
        """
        raise NotImplementedError("method 'setDefaultLocator' not implemented")

    def getPluginManager(self):
        """
            Get the plug-in manager for this communicator.
        Returns: This communicator's plug-in manager.
        """
        raise NotImplementedError("method 'getPluginManager' not implemented")

    def getValueFactoryManager(self):
        """
            Get the value factory manager for this communicator.
        Returns: This communicator's value factory manager.
        """
        raise NotImplementedError("method 'getValueFactoryManager' not implemented")

    def flushBatchRequests(self, compress):
        """
            Flush any pending batch requests for this communicator. This means all batch requests invoked on fixed proxies
            for all connections associated with the communicator. Any errors that occur while flushing a connection are
            ignored.
        Arguments:
        compress -- Specifies whether or not the queued batch requests should be compressed before being sent over the wire.
        """
        raise NotImplementedError("method 'flushBatchRequests' not implemented")

    def createAdmin(self, adminAdapter, adminId):
        """
            Add the Admin object with all its facets to the provided object adapter. If Ice.Admin.ServerId is
            set and the provided object adapter has a Locator, createAdmin registers the Admin's Process facet with
            the Locator's LocatorRegistry. createAdmin must only be called once; subsequent calls raise
            InitializationException.
        Arguments:
        adminAdapter -- The object adapter used to host the Admin object; if null and Ice.Admin.Endpoints is set, create, activate and use the Ice.Admin object adapter.
        adminId -- The identity of the Admin object.
        Returns: A proxy to the main ("") facet of the Admin object. Never returns a null proxy.
        """
        raise NotImplementedError("method 'createAdmin' not implemented")

    def getAdmin(self):
        """
            Get a proxy to the main facet of the Admin object. getAdmin also creates the Admin object and creates and
            activates the Ice.Admin object adapter to host this Admin object if Ice.Admin.Enpoints is set. The identity of
            the Admin object created by getAdmin is {value of Ice.Admin.InstanceName}/admin, or {UUID}/admin when
            Ice.Admin.InstanceName is not set. If Ice.Admin.DelayCreation is 0 or not set, getAdmin is called
            by the communicator initialization, after initialization of all plugins.
        Returns: A proxy to the main ("") facet of the Admin object, or a null proxy if no Admin object is configured.
        """
        raise NotImplementedError("method 'getAdmin' not implemented")

    def addAdminFacet(self, servant, facet):
        """
            Add a new facet to the Admin object. Adding a servant with a facet that is already registered throws
            AlreadyRegisteredException.
        Arguments:
        servant -- The servant that implements the new Admin facet.
        facet -- The name of the new Admin facet.
        """
        raise NotImplementedError("method 'addAdminFacet' not implemented")

    def removeAdminFacet(self, facet):
        """
            Remove the following facet to the Admin object. Removing a facet that was not previously registered throws
            NotRegisteredException.
        Arguments:
        facet -- The name of the Admin facet.
        Returns: The servant associated with this Admin facet.
        """
        raise NotImplementedError("method 'removeAdminFacet' not implemented")

    def findAdminFacet(self, facet):
        """
            Returns a facet of the Admin object.
        Arguments:
        facet -- The name of the Admin facet.
        Returns: The servant associated with this Admin facet, or null if no facet is registered with the given name.
        """
        raise NotImplementedError("method 'findAdminFacet' not implemented")

    def findAllAdminFacets(self):
        """
            Returns a map of all facets of the Admin object.
        Returns: A collection containing all the facet names and servants of the Admin object.
        """
        raise NotImplementedError("method 'findAllAdminFacets' not implemented")
