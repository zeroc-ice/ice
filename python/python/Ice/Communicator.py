# Copyright (c) ZeroC, Inc.

from .ObjectAdapter import ObjectAdapter
from .ImplicitContext import ImplicitContext
from .Properties import Properties
from ._LoggerI import LoggerI
from .Logger import Logger
from typing import final
from .Future import Future

@final
class Communicator:
    """
    The main entry point to the Ice runtime, represented by the `Ice.Communicator` class.

    Example
    -------
    The following example shows how to create a communicator and use the ``async with`` statement to ensure that the
    communicator is properly destroyed.

    .. code-block:: python

        async def main():
            async with Ice.initialize(
                sys.argv,
                eventLoop=asyncio.get_running_loop()) as communicator:
                ...

        if __name__ == "__main__":
            asyncio.run(main())
    """

    def __init__(self, impl, eventLoopAdapter=None):
        self._impl = impl
        impl._setWrapper(self)
        self._eventLoopAdapter = eventLoopAdapter

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        self._impl.destroy()

    async def __aenter__(self):
        return self

    async def __aexit__(self, type, value, traceback):
        await self.destroyAsync()

    @property
    def eventLoopAdapter(self):
        """
        The event loop adapter associated with this communicator, or None if the communicator does not have one.

        Returns
        -------
        Ice.EventLoopAdapter or None
            The event loop adapter used for integrating Ice with a custom event loop.
        """

        return self._eventLoopAdapter

    def _getImpl(self):
        return self._impl

    def destroy(self):
        """
        Destroy the communicator. This operation calls shutdown implicitly. Calling destroy cleans up
        memory, and shuts down this communicator's client functionality and destroys all object adapters. Subsequent
        calls to destroy are ignored.
        """
        self._impl.destroy()

    def destroyAsync(self):
        """
        Asynchronously destroy the communicator. This operation calls shutdown implicitly. Calling destroy cleans up
        memory, and shuts down this communicator's client functionality and destroys all object adapters. Subsequent
        calls to destroy are ignored.
        """
        future = Future()
        def completed():
            future.set_result(None)

        wrappedFuture = future
        if self._eventLoopAdapter:
            wrappedFuture = self._eventLoopAdapter.wrapFuture(wrappedFuture)

        self._impl.destroyAsync(completed)
        return wrappedFuture

    def shutdown(self):
        """
        Shuts down this communicator's server functionality, which includes the deactivation of all object adapters.
        Attempts to use a deactivated object adapter raise ObjectAdapterDeactivatedException. Subsequent calls to
        shutdown are ignored.
        After shutdown returns, no new requests are processed. However, requests that have been started before shutdown
        was called might still be active. You can use waitForShutdown to wait for the completion of all
        requests.
        """
        self._impl.shutdown()

    def waitForShutdown(self):
        """
        Wait until the application has called shutdown (or destroy). On the server side, this
        operation blocks the calling thread until all currently-executing operations have completed. On the client
        side, the operation simply blocks until another thread has called shutdown or destroy.
        A typical use of this operation is to call it from the main thread, which then waits until some other thread
        calls shutdown. After shut-down is complete, the main thread returns and can do some cleanup work
        before it finally calls destroy to shut down the client functionality, and then exits the application.
        """
        # If invoked by the main thread, waitForShutdown only blocks for the specified timeout in order to give us a
        # chance to handle signals.
        while not self._impl.waitForShutdown(500):
            pass

    def shutdownCompleted(self):
        """
        Return a Future that is marked as done when the communicator's shutdown completes.

        The returned Future always completes successfully.

        Returns
        -------
        Ice.Future
            A Future that is marked as done upon shutdown completion.
        """
        return self._impl.shutdownCompleted()

    def isShutdown(self):
        """
        Check whether the communicator has been shut down.

        Returns
        -------
        bool
            True if the communicator has been shut down; False otherwise.
        """
        return self._impl.isShutdown()

    def stringToProxy(self, str):
        """
        Convert a proxy string representation into a proxy.

        For example, `MyCategory/MyObject:tcp -h some_host -p 10000` creates a proxy that refers to the Ice object
        having an identity with a name "MyObject" and a category "MyCategory", with the server running on host "some_host",
        port 10000. If the proxy string representation does not parse correctly, the operation throws ParseException.
        Refer to the Ice manual for a detailed description of the syntax supported by proxy strings.

        Parameters
        ----------
        proxyString : str
            The proxy string representation to convert into a proxy.

        Returns
        -------
        Ice.ObjectPrx
            The proxy, or None if the string is empty.

        Raises
        ------
        ParseException
            If the proxy string representation does not parse correctly.
        """
        return self._impl.stringToProxy(str)

    def proxyToString(self, proxy):
        """
        Convert a proxy into a string.

        Parameters
        ----------
        proxy : Ice.ObjectPrx
            The proxy to convert into a string representation.

        Returns
        -------
        str
            The proxy string representation, or an empty string if the proxy is None.
        """
        return self._impl.proxyToString(proxy)

    def propertyToProxy(self, property):
        """
        Convert a set of proxy properties into a proxy.

        The "base" name supplied in the property argument refers to a property containing a stringified proxy,
        such as `MyProxy=id:tcp -h localhost -p 10000`. Additional properties configure local settings for the proxy,
        such as `MyProxy.PreferSecure=1`. The "Properties" appendix in the Ice manual describes each of the supported
        proxy properties.

        Parameters
        ----------
        property : str
            The base property name.

        Returns
        -------
        Ice.ObjectPrx
            The proxy.
        """
        return self._impl.propertyToProxy(property)

    def proxyToProperty(self, proxy, property):
        """
        Convert a proxy to a set of properties.

        Parameters
        ----------
        proxy : Ice.ObjectPrx
            The proxy to convert.
        property : str
            The base property name.

        Returns
        -------
        dict
            The property set.
        """
        return self._impl.proxyToProperty(proxy, property)

    def identityToString(self, identity):
        """
        Convert an identity into a string.

        Parameters
        ----------
        identity : Ice.Identity
            The identity to convert into a string.

        Returns
        -------
        str
            The string representation of the identity.
        """
        return self._impl.identityToString(identity)

    def createObjectAdapter(self, name):
        """
        Create a new object adapter.

        The endpoints for the object adapter are taken from the property ``name.Endpoints``. It is legal to create an
        object adapter with an empty string as its name. Such an object adapter is accessible via bidirectional connections
        or by collocated invocations that originate from the same communicator as is used by the adapter. Attempts to create
        a named object adapter for which no configuration can be found raise an InitializationException.

        Parameters
        ----------
        name : str
            The object adapter name.

        Returns
        -------
        Ice.ObjectAdapter
            The new object adapter.
        """
        adapter = self._impl.createObjectAdapter(name)
        return ObjectAdapter(adapter)

    def createObjectAdapterWithEndpoints(self, name, endpoints):
        """
        Create a new object adapter with endpoints.

        This operation sets the property `name.Endpoints` and then calls `createObjectAdapter`. It is provided as a
        convenience function. Calling this operation with an empty name will result in a UUID being generated for the name.

        Parameters
        ----------
        name : str
            The object adapter name.
        endpoints : str
            The endpoints for the object adapter.

        Returns
        -------
        Ice.ObjectAdapter
            The new object adapter.
        """
        adapter = self._impl.createObjectAdapterWithEndpoints(name, endpoints)
        return ObjectAdapter(adapter)

    def createObjectAdapterWithRouter(self, name, router):
        """
        Create a new object adapter with a router.

        This operation creates a routed object adapter. Calling this operation with an empty name will result in a UUID being generated for the name.

        Parameters
        ----------
        name : str
            The object adapter name.
        router : Ice.RouterPrx
            The router.

        Returns
        -------
        Ice.ObjectAdapter
            The new object adapter.
        """
        adapter = self._impl.createObjectAdapterWithRouter(name, router)
        return ObjectAdapter(adapter)

    def getDefaultObjectAdapter(self):
        """
        Get the object adapter that is associated by default with new outgoing connections created by this
        communicator. This method returns None unless you set a default object adapter using createDefaultObjectAdapter.

        Returns
        -------
        Ice.ObjectAdapter or None
            The object adapter associated by default with new outgoing connections.
        """
        return self._impl.getDefaultObjectAdapter()

    def setDefaultObjectAdapter(self, adapter):
        """
        Set the object adapter that is associated by default with new outgoing connections created by this communicator.

        Parameters
        ----------
        adapter : Ice.ObjectAdapter or None
            The object adapter to associate with new outgoing connections.
        """
        self._impl.setDefaultObjectAdapter(adapter)

    def getImplicitContext(self):
        """
        Get the implicit context associated with this communicator.

        Returns
        -------
        Ice.ImplicitContext or None
            The implicit context associated with this communicator, or None if the property Ice.ImplicitContext is not set or is set to None.
        """
        context = self._impl.getImplicitContext()
        if context is None:
            return None
        else:
            return ImplicitContext(context)

    def getProperties(self):
        """
        Get the properties for this communicator.

        Returns
        -------
        Ice.Properties
            The properties associated with this communicator.
        """
        properties = self._impl.getProperties()
        return Properties(properties)

    def getLogger(self):
        """
        Get the logger for this communicator.

        Returns
        -------
        Ice.Logger
            The logger associated with this communicator.
        """
        logger = self._impl.getLogger()
        if isinstance(logger, Logger):
            return logger
        else:
            return LoggerI(logger)

    def getDefaultRouter(self):
        """
        Get the default router for this communicator.

        Returns
        -------
        Ice.RouterPrx or None
            The default router for this communicator, or None if no default router has been set.
        """
        return self._impl.getDefaultRouter()

    def setDefaultRouter(self, router):
        """
        Set a default router for this communicator.

        All newly created proxies will use this default router. To disable the default router, pass `None`.
        Note that this operation has no effect on existing proxies.

        You can also set a router for an individual proxy by calling the method `Ice.ObjectPrx.ice_router` on the
        proxy.

        Parameters
        ----------
        router : Ice.RouterPrx or None
            The default router to use for this communicator.
        """
        self._impl.setDefaultRouter(router)

    def getDefaultLocator(self):
        """
        Get the default locator for this communicator.

        Returns
        -------
        Ice.LocatorPrx or None
            The default locator for this communicator, or None if no default locator has been set.
        """
        return self._impl.getDefaultLocator()

    def setDefaultLocator(self, locator):
        """
        Set a default Ice locator for this communicator.

        All newly created proxies and object adapters will use this default locator. To disable the default locator,
        pass `None`. Note that this operation has no effect on existing proxies or object adapters.

        You can also set a locator for an individual proxy by calling the method :any:`Ice.ObjectPrx.ice_locator` on the
        proxy, or for an object adapter by calling :any:`ObjectAdapter.setLocator` on the object adapter.

        Parameters
        ----------
        locator : Ice.LocatorPrx or None
            The default locator to use for this communicator.
        """
        self._impl.setDefaultLocator(locator)

    def flushBatchRequests(self, compress):
        """
        Flush any pending batch requests for this communicator. This means all batch requests invoked on fixed proxies
        for all connections associated with the communicator. Any errors that occur while flushing a connection are
        ignored.

        Parameters
        ----------
        compress : bool
            Specifies whether or not the queued batch requests should be compressed before being sent over the wire.
        """
        self._impl.flushBatchRequests(compress)

    def flushBatchRequestsAsync(self, compress):
        """
        TODO fix async description
        Flush any pending batch requests for this communicator. This means all batch requests invoked on fixed proxies
        for all connections associated with the communicator. Any errors that occur while flushing a connection are
        ignored.

        Parameters
        ----------
        compress : bool
            Specifies whether or not the queued batch requests should be compressed before being sent over the wire.
        """
        return self._impl.flushBatchRequestsAsync(compress)

    def createAdmin(self, adminAdapter, adminId):
        """
        Add the Admin object with all its facets to the provided object adapter.

        If ``Ice.Admin.ServerId`` is set and the provided object adapter has a Locator, ``createAdmin`` registers the
        Admin's Process facet with the Locator's LocatorRegistry. ``createAdmin`` must only be called once;
        subsequent calls raise InitializationException.

        Parameters
        ----------
        adminAdapter : Ice.ObjectAdapter or None
            The object adapter used to host the Admin object. If `None` and `Ice.Admin.Endpoints` is set,
            create, activate, and use the Ice.Admin object adapter.
        adminId : Ice.Identity
            The identity of the Admin object.

        Returns
        -------
        Ice.ObjectPrx
            A proxy to the main ("") facet of the Admin object.

        Raises
        ------
        InitializationException
            If the method is called more than once.
        """
        return self._impl.createAdmin(adminAdapter, adminId)

    def getAdmin(self):
        """
        Get a proxy to the main facet of the Admin object.

        This method also creates the Admin object and activates the ``Ice.Admin`` object adapter to host this Admin
        object if ``Ice.Admin.Endpoints`` property is set. The identity of the Admin object created by getAdmin is
        ``{value of Ice.Admin.InstanceName}/admin``, or ``{UUID}/admin`` when ``Ice.Admin.InstanceName`` property is
        not set. If ``Ice.Admin.DelayCreation`` property is 0 or not set, getAdmin is called by the communicator
        initialization, after initialization of all plugins.

        Returns
        -------
        Ice.ObjectPrx or None
            A proxy to the main ("") facet of the Admin object, or None if no Admin object is configured.
        """
        return self._impl.getAdmin()

    def addAdminFacet(self, servant, facet):
        """
        Adds a new facet to the Admin object.

        This method adds a new servant implementing the specified Admin facet.

        Parameters
        ----------
        servant : Ice.Object
            The servant that implements the new Admin facet.
        facet : str
            The name of the new Admin facet.

        Raises
        ------
        AlreadyRegisteredException
            If the facet is already registered.
        """
        self._impl.addAdminFacet(servant, facet)

    def removeAdminFacet(self, facet):
        """
        Remove the specified facet from the Admin object.

        Removing a facet that was not previously registered throws a NotRegisteredException.

        Parameters
        ----------
        facet : str
            The name of the Admin facet to be removed.

        Returns
        -------
        Ice.Object
            The servant associated with the removed Admin facet.

        Raises
        ------
        NotRegisteredException
            If the facet is not registered.
        """
        return self._impl.removeAdminFacet(facet)

    def findAdminFacet(self, facet):
        """
        Return a facet of the Admin object.

        Parameters
        ----------
        facet : str
            The name of the Admin facet.

        Returns
        -------
        Ice.Object or None
            The servant associated with the specified Admin facet, or None if no facet is registered with the given name.
        """
        return self._impl.findAdminFacet(facet)

    def findAllAdminFacets(self):
        """
        Return a dictionary of all facets of the Admin object.

        Returns
        -------
        dict[str, Object]
            A dictionary where the keys are facet names and the values are the associated servants.
        """
        return self._impl.findAllAdminFacets()
