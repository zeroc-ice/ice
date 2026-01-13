# Copyright (c) ZeroC, Inc.

from __future__ import annotations

import asyncio
from typing import TYPE_CHECKING, Self, final, overload

import IcePy

from ._LoggerI import LoggerI
from .asyncio.EventLoopAdapter import EventLoopAdapter as AsyncIOEventLoopAdapter
from .Future import Future
from .ImplicitContext import ImplicitContext
from .InitializationData import InitializationData
from .Logger import Logger
from .ObjectAdapter import ObjectAdapter
from .Properties import Properties

if TYPE_CHECKING:
    from collections.abc import Awaitable

    from .CompressBatch import CompressBatch
    from .EventLoopAdapter import EventLoopAdapter
    from .Identity import Identity
    from .Locator import LocatorPrx
    from .Object import Object
    from .ObjectPrx import ObjectPrx
    from .Router import RouterPrx


@final
class Communicator:
    """
    Communicator is the central object in Ice. Its responsibilities include:
    - creating and managing outgoing connections
    - executing callbacks in its client thread pool
    - creating and destroying object adapters
    - loading plug-ins
    - managing properties (configuration), retries, logging, instrumentation, and more.
    A communicator is usually the first object you create when programming with Ice.
    You can create multiple communicators in a single program, but this is not common.

    Example
    -------
    The following example shows how to create a communicator and use the ``async with`` statement to ensure that the
    communicator is properly destroyed.

    .. code-block:: python

        async def main():
            async with Ice.Communicator(
                sys.argv, eventLoop=asyncio.get_running_loop()
            ) as communicator:
                ...


        if __name__ == "__main__":
            asyncio.run(main())
    """

    @overload
    def __init__(self, args: list[str] | None = None, eventLoop: asyncio.AbstractEventLoop | None = None) -> None: ...

    @overload
    def __init__(self, *, initData: InitializationData | None = None) -> None: ...

    def __init__(
        self,
        args: list[str] | None = None,
        eventLoop: asyncio.AbstractEventLoop | None = None,
        initData: InitializationData | None = None,
    ) -> None:
        """
        Initializes a new instance of Communicator.

        Parameters
        ----------
        args : list[str] | None, optional
            The command-line arguments, parsed into Ice properties by this function.
        eventLoop : asyncio.AbstractEventLoop | None, optional
            An asyncio event loop used to run coroutines and wrap futures. If provided, a new event loop adapter is
            created and configured with the communicator. This adapter is responsible for executing coroutines returned
            by Ice asynchronous dispatch methods and for wrapping Ice futures (from Ice Async APIs) into asyncio
            futures. This argument and the `initData` argument are mutually exclusive. If the `initData` argument is
            provided, the event loop adapter can be set using the :attr:`InitializationData.eventLoopAdapter` attribute.
        initData : InitializationData | None, optional
            Options for the new communicator. This argument and the `args` argument are mutually exclusive.
        """
        eventLoopAdapter = None
        if initData:
            eventLoopAdapter = initData.eventLoopAdapter
        elif eventLoop:
            eventLoopAdapter = AsyncIOEventLoopAdapter(eventLoop)

        if args:
            initData = InitializationData(properties=Properties(args))

        # initData can be None here, which is acceptable.
        self._impl = IcePy.Communicator(initData)
        self._impl._setWrapper(self)
        self._eventLoopAdapter = eventLoopAdapter

    def __enter__(self) -> Self:
        return self

    def __exit__(self, type, value, traceback) -> None:  # type: ignore
        self._impl.destroy()

    async def __aenter__(self) -> Self:
        return self

    async def __aexit__(self, type, value, traceback) -> None:  # type: ignore
        await self.destroyAsync()

    @property
    def eventLoopAdapter(self) -> EventLoopAdapter | None:
        """
        Returns this communicator's event loop adapter, or ``None`` if the communicator does not have one.

        Returns
        -------
        EventLoopAdapter | None
            The event loop adapter used by this communicator.
        """

        return self._eventLoopAdapter

    def _getImpl(self) -> IcePy.Communicator:
        return self._impl

    def destroy(self) -> None:
        """
        Destroys this communicator. This function calls :func:`shutdown` implicitly. Calling this function destroys all
        object adapters, and closes all outgoing connections. This function waits for all outstanding dispatches to
        complete before returning. This includes "bidirectional dispatches" that execute on outgoing connections.
        """
        self._impl.destroy()

    def destroyAsync(self) -> Awaitable[None]:
        """
        Destroys this communicator asynchronously.

        See :func:`destroy`.
        """
        future = Future()

        def completed() -> None:
            future.set_result(None)

        wrappedFuture = future
        if self._eventLoopAdapter:
            wrappedFuture = self._eventLoopAdapter.wrapFuture(wrappedFuture)

        self._impl.destroyAsync(completed)
        return wrappedFuture

    def shutdown(self) -> None:
        """
        Shuts down this communicator. This function calls :func:`ObjectAdapter.deactivate` on all object adapters
        created by this communicator. Shutting down a communicator has no effect on outgoing connections.
        """
        self._impl.shutdown()

    def waitForShutdown(self) -> None:
        """
        Waits for shutdown to complete. This function calls :func:`ObjectAdapter.waitForDeactivate` on all object
        adapters created by this communicator. In a client application that does not accept incoming connections, this
        function returns as soon as another thread calls :func:`shutdown` or :func:`destroy` on this communicator.
        """
        # If invoked by the main thread, waitForShutdown only blocks for the specified timeout in order to give us a
        # chance to handle signals.
        while not self._impl.waitForShutdown(500):
            pass

    def shutdownCompleted(self) -> Awaitable[None]:
        """
        Returns an :class:`Awaitable` that completes when the communicator's shutdown completes.
        This task always completes successfully.

        Notes
        -----
        The shutdown of a communicator completes when all its incoming connections are closed.
        Awaiting this task is equivalent to awaiting :func:`waitForShutdown`.

        Returns
        -------
        Awaitable[None]
            An awaitable that completes upon shutdown completion.
        """
        return self._impl.shutdownCompleted()

    def isShutdown(self) -> bool:
        """
        Checks whether or not :func:`shutdown` was called on this communicator.

        Returns
        -------
        bool
            ``True`` if :func:`shutdown` was called on this communicator, ``False`` otherwise
        """
        return self._impl.isShutdown()

    def stringToProxy(self, str: str) -> ObjectPrx | None:
        """
        Converts a stringified proxy into a proxy.

        Parameters
        ----------
        str : str
            The stringified proxy to convert into a proxy.

        Returns
        -------
        ObjectPrx | None
            The proxy, or ``None`` if ``str`` is an empty string.

        Raises
        ------
        ParseException
            If ``str`` is not a valid proxy string.
        """
        return self._impl.stringToProxy(str)

    def proxyToString(self, proxy: ObjectPrx | None) -> str:
        """
        Converts a proxy into a string.

        Parameters
        ----------
        proxy : ObjectPrx | None
            The proxy to convert into a stringified proxy.

        Returns
        -------
        str
            The stringified proxy, or an empty string if ``proxy`` is ``None``.
        """
        return self._impl.proxyToString(proxy)

    def propertyToProxy(self, property: str) -> ObjectPrx | None:
        """
        Converts a set of proxy properties into a proxy. The "base" name supplied in the ``property`` argument refers
        to a property containing a stringified proxy, such as ``MyProxy=id:tcp -h localhost -p 10000``.
        Additional properties configure local settings for the proxy.

        Parameters
        ----------
        property : str
            The base property name.

        Returns
        -------
        ObjectPrx | None
            The proxy, or ``None`` if the property is not set.
        """
        return self._impl.propertyToProxy(property)

    def proxyToProperty(self, proxy: ObjectPrx, property: str) -> dict[str, str]:
        """
        Converts a proxy into a set of proxy properties.

        Parameters
        ----------
        proxy : ObjectPrx
            The proxy.
        property : str
            The base property name.

        Returns
        -------
        dict[str, str]
            The property set.
        """
        return self._impl.proxyToProperty(proxy, property)

    def identityToString(self, identity: Identity) -> str:
        """
        Converts an identity into a string.

        Parameters
        ----------
        identity : Identity
            The identity to convert into a string.

        Returns
        -------
        str
            The "stringified" identity.
        """
        return self._impl.identityToString(identity)

    def createObjectAdapter(self, name: str) -> ObjectAdapter:
        """
        Creates a new object adapter. The endpoints for the object adapter are taken from the property
        ``name.Endpoints``.

        It is legal to create an object adapter with the empty string as its name. Such an object adapter is
        accessible via bidirectional connections or by collocated invocations.

        Parameters
        ----------
        name : str
            The object adapter name.

        Returns
        -------
        ObjectAdapter
            The new object adapter.
        """
        adapter = self._impl.createObjectAdapter(name)
        return ObjectAdapter(adapter)

    def createObjectAdapterWithEndpoints(self, name: str, endpoints: str) -> ObjectAdapter:
        """
        Creates a new object adapter with endpoints. This function sets the property ``name.Endpoints``,
        and then calls :func:`createObjectAdapter`. It is provided as a convenience function. Calling this function
        with an empty name will result in a UUID being generated for the name.

        Parameters
        ----------
        name : str
            The object adapter name.
        endpoints : str
            The endpoints of the object adapter.

        Returns
        -------
        ObjectAdapter
            The new object adapter.
        """
        adapter = self._impl.createObjectAdapterWithEndpoints(name, endpoints)
        return ObjectAdapter(adapter)

    def createObjectAdapterWithRouter(self, name: str, router: RouterPrx) -> ObjectAdapter:
        """
        Creates a new object adapter with a router. This function creates a routed object adapter.
        Calling this function with an empty name will result in a UUID being generated for the name.

        Parameters
        ----------
        name : str
            The object adapter name.
        router : RouterPrx
            The router.

        Returns
        -------
        ObjectAdapter
            The new object adapter.
        """
        adapter = self._impl.createObjectAdapterWithRouter(name, router)
        return ObjectAdapter(adapter)

    def getDefaultObjectAdapter(self) -> ObjectAdapter | None:
        """
        Gets the object adapter that is associated by default with new outgoing connections created by this
        communicator. This function returns ``None`` unless you set a non-``None`` default object adapter using
        :func:`setDefaultObjectAdapter`.

        Returns
        -------
        ObjectAdapter | None
            The object adapter associated by default with new outgoing connections.

        Raises
        ------
        CommunicatorDestroyedException
            If the communicator has been destroyed.
        """
        return self._impl.getDefaultObjectAdapter()

    def setDefaultObjectAdapter(self, adapter: ObjectAdapter | None):
        """
        Sets the object adapter that will be associated with new outgoing connections created by this communicator.
        This function has no effect on existing outgoing connections, or on incoming connections.

        Parameters
        ----------
        adapter : ObjectAdapter | None
            The object adapter to associate with new outgoing connections.
        """
        self._impl.setDefaultObjectAdapter(adapter)

    def getImplicitContext(self) -> ImplicitContext | None:
        """
        Gets the implicit context associated with this communicator.

        Returns
        -------
        ImplicitContext | None
            The implicit context associated with this communicator; returns ``None`` when the property
            ``Ice.ImplicitContext`` is not set or is set to ``None``.
        """
        context = self._impl.getImplicitContext()
        if context is None:
            return None
        else:
            return ImplicitContext(context)

    def getProperties(self) -> Properties:
        """
        Gets the properties of this communicator.

        Returns
        -------
        Properties
            This communicator's properties.
        """
        properties = self._impl.getProperties()
        return Properties(properties=properties)

    def getLogger(self) -> Logger:
        """
        Gets the logger of this communicator.

        Returns
        -------
        Logger
            This communicator's logger.
        """
        logger = self._impl.getLogger()
        if isinstance(logger, Logger):
            return logger
        else:
            return LoggerI(logger)

    def getDefaultRouter(self) -> RouterPrx | None:
        """
        Gets the default router of this communicator.

        Returns
        -------
        RouterPrx | None
            The default router of this communicator, or ``None`` if no default router has been set.

        Raises
        ------
        CommunicatorDestroyedException
            If the communicator has been destroyed.
        """
        return self._impl.getDefaultRouter()

    def setDefaultRouter(self, router: RouterPrx | None):
        """
        Sets the default router of this communicator. All newly created proxies will use this default router.
        This function has no effect on existing proxies.

        Notes
        -----
        You can set a router for an individual proxy by calling :func:`ObjectPrx.ice_router` on the proxy.

        Parameters
        ----------
        router : RouterPrx | None
            The new default router. Use ``None`` to remove the default router.
        """
        self._impl.setDefaultRouter(router)

    def getDefaultLocator(self) -> LocatorPrx | None:
        """
        Gets the default locator of this communicator.

        Returns
        -------
        LocatorPrx | None
            The default locator of this communicator, or ``None`` if no default locator has been set.
        """
        return self._impl.getDefaultLocator()

    def setDefaultLocator(self, locator: LocatorPrx | None):
        """
        Sets the default locator of this communicator. All newly created proxies will use this default locator.
        This function has no effect on existing proxies or object adapters.

        Notes
        -----
        You can set a locator for an individual proxy by calling :func:`ObjectPrx.ice_locator` on the proxy,
        or for an object adapter by calling :func:`ObjectAdapter.setLocator` on the object adapter.

        Parameters
        ----------
        locator : LocatorPrx | None
            The new default locator. Use ``None`` to remove the default locator.
        """
        self._impl.setDefaultLocator(locator)

    def flushBatchRequests(self, compress: CompressBatch):
        """
        Flushes any pending batch requests of this communicator. This means all batch requests invoked on fixed
        proxies for all connections associated with the communicator. Errors that occur while flushing a connection
        are ignored.

        Parameters
        ----------
        compress : CompressBatch
            Specifies whether or not the queued batch requests should be compressed before being sent over the wire.
        """
        self._impl.flushBatchRequests(compress)

    def flushBatchRequestsAsync(self, compress: CompressBatch) -> Awaitable[None]:
        """
        Flushes any pending batch requests of this communicator. This means all batch requests invoked on fixed
        proxies for all connections associated with the communicator. Errors that occur while flushing a connection
        are ignored.

        Parameters
        ----------
        compress : CompressBatch
            Specifies whether or not the queued batch requests should be compressed before being sent over the wire.

        Returns
        -------
        Awaitable[None]
            An :class:`Awaitable` that completes when all batch requests have been sent.
        """
        return self._impl.flushBatchRequestsAsync(compress)

    def createAdmin(self, adminAdapter: ObjectAdapter | None, adminId: Identity) -> ObjectPrx:
        """
        Adds the Admin object with all its facets to the provided object adapter.
        If ``Ice.Admin.ServerId`` is set and the provided object adapter has a :class:`Locator`,
        this function registers the Admin's Process facet with the :class:`Locator`'s :class:`LocatorRegistry`.

        Parameters
        ----------
        adminAdapter : ObjectAdapter | None
            The object adapter used to host the Admin object; if it is ``None`` and ``Ice.Admin.Endpoints`` is set,
            this function uses the ``Ice.Admin`` object adapter, after creating and activating this adapter.
        adminId : Identity
            The identity of the Admin object.

        Returns
        -------
        ObjectPrx
            A proxy to the main ("") facet of the Admin object.

        Raises
        ------
        InitializationException
            If this function is called more than once.
        """
        return self._impl.createAdmin(adminAdapter, adminId)

    def getAdmin(self) -> ObjectPrx | None:
        """
        Gets a proxy to the main facet of the Admin object.

        ``getAdmin`` also creates the Admin object and creates and activates the ``Ice.Admin`` object adapter to host
        this Admin object if ``Ice.Admin.Endpoints`` is set. The identity of the Admin object created by ``getAdmin``
        is ``{value of Ice.Admin.InstanceName}/admin``, or ``{UUID}/admin`` when ``Ice.Admin.InstanceName`` is not set.
        If ``Ice.Admin.DelayCreation`` is ``0`` or not set, ``getAdmin`` is called by the communicator initialization,
        after initialization of all plugins.

        Returns
        -------
        ObjectPrx | None
            A proxy to the main ("") facet of the Admin object, or ``None`` if no Admin object is configured.

        Raises
        ------
        CommunicatorDestroyedException
            If the communicator has been destroyed.
        """
        return self._impl.getAdmin()

    def addAdminFacet(self, servant: Object | None, facet: str) -> None:
        """
        Adds a new facet to the Admin object.

        Parameters
        ----------
        servant : Object
            The servant that implements the new Admin facet.
        facet : str
            The name of the new Admin facet.

        Raises
        ------
        AlreadyRegisteredException
            If a facet with the same name is already registered.
        """
        self._impl.addAdminFacet(servant, facet)

    def removeAdminFacet(self, facet: str) -> Object:
        """
        Removes a facet from the Admin object.

        Parameters
        ----------
        facet : str
            The name of the Admin facet.

        Returns
        -------
        Object
            The servant associated with this Admin facet.

        Raises
        ------
        NotRegisteredException
            If no facet with the given name is registered.
        """
        return self._impl.removeAdminFacet(facet)

    def findAdminFacet(self, facet: str) -> Object | None:
        """
        Returns a facet of the Admin object.

        Parameters
        ----------
        facet : str
            The name of the Admin facet.

        Returns
        -------
        Object | None
            The servant associated with this Admin facet, or ``None`` if no facet is registered with the given name.
        """
        return self._impl.findAdminFacet(facet)

    def findAllAdminFacets(self) -> dict[str, Object]:
        """
        Returns a map of all facets of the Admin object.

        Returns
        -------
        dict[str, Object]
            A collection containing all the facet names and servants of the Admin object.
        """
        return self._impl.findAllAdminFacets()


__all__ = ["Communicator"]
