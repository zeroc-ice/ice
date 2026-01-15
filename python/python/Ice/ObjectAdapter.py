# Copyright (c) ZeroC, Inc.

# Avoid evaluating annotations at function definition time.
from __future__ import annotations

from typing import TYPE_CHECKING, final

import IcePy

if TYPE_CHECKING:
    from .Communicator import Communicator
    from .IcePyTypes import Endpoint
    from .Identity import Identity
    from .Locator import LocatorPrx
    from .Object import Object
    from .ObjectPrx import ObjectPrx
    from .ServantLocator import ServantLocator


@final
class ObjectAdapter:
    """
    An object adapter is the main server-side Ice API. It has two main purposes:
    - accept incoming connections from clients and dispatch requests received over these connections (see
      :func:`activate`); and
    - maintain servants that handle the requests (see :func:`add`, :func:`addDefaultServant`).

    An object adapter can dispatch "bidirectional requests"--requests it receives over an outgoing connection
    instead of a more common incoming connection. It can also dispatch collocated requests (with no connection at all).
    """

    def __init__(self, impl: IcePy.ObjectAdapter):
        self._impl = impl

    def getName(self) -> str:
        """
        Gets the name of this object adapter.

        Returns
        -------
        str
            This object adapter's name.
        """
        return self._impl.getName()

    def getCommunicator(self) -> Communicator:
        """
        Gets the communicator that created this object adapter.

        Returns
        -------
        Communicator
            This object adapter's communicator.
        """
        communicator = self._impl.getCommunicator()
        return communicator._getWrapper()

    def activate(self) -> None:
        """
        Starts receiving and dispatching requests received over incoming connections.

        Notes
        -----
        When this object adapter is an indirect object adapter configured with a locator proxy, this
        function also registers the object adapter's published endpoints with this locator.
        """
        self._impl.activate()

    def hold(self) -> None:
        """
        Stops reading requests from incoming connections. Outstanding dispatches are not affected.
        The object adapter can be reactivated with :func:`activate`.

        Notes
        -----
        This function is provided for backward compatibility with older versions of Ice.
        Don't use it in new applications.
        """
        self._impl.hold()

    def waitForHold(self) -> None:
        """
        Waits until the object adapter is in the holding state (see :func:`hold`) and the dispatch of requests received
        over incoming connections has completed.

        Notes
        -----
        This function is provided for backward compatibility with older versions of Ice.
        Don't use it in new applications.
        """
        # If invoked by the main thread, waitForHold only blocks for the specified timeout in order to give us a chance
        # to handle signals.
        while not self._impl.waitForHold(500):
            pass

    def deactivate(self) -> None:
        """
        Deactivates this object adapter: stops accepting new connections from clients and closes gracefully all
        incoming connections created by this object adapter once all outstanding dispatches have completed.
        If this object adapter is indirect, this function also unregisters the object adapter from the locator
        (see :func:`activate`).

        This function does not cancel outstanding dispatches: it lets them execute until completion.
        A deactivated object adapter cannot be reactivated again; it can only be destroyed.
        """
        self._impl.deactivate()

    def waitForDeactivate(self) -> None:
        """
        Waits until :func:`deactivate` is called on this object adapter and all connections accepted by this object adapter
        are closed. A connection is closed only after all outstanding dispatches on this connection have completed.
        """
        # If invoked by the main thread, waitForDeactivate only blocks for the specified timeout in order to give us a
        # chance to handle signals.
        while not self._impl.waitForDeactivate(500):
            pass

    def isDeactivated(self) -> bool:
        """
        Checks whether or not :func:`deactivate` was called on this object adapter.

        Returns
        -------
        bool
            ``True`` if :func:`deactivate` has been called on this object adapter, ``False`` otherwise.
        """
        return self._impl.isDeactivated()

    def destroy(self) -> None:
        """
        Destroys this object adapter and cleans up all resources associated with it.
        Once this function has returned, you can recreate another object adapter with the same name.
        """
        self._impl.destroy()

    def add(self, servant: Object, id: Identity) -> ObjectPrx:
        """
        Adds a servant to this object adapter's Active Servant Map (ASM).
        The ASM is a map {identity, facet} -> servant.

        Notes
        -----
        This function is equivalent to calling :func:`addFacet` with an empty facet.

        Parameters
        ----------
        servant : Object
            The servant to add.
        id : Identity
            The identity of the Ice object that is implemented by the servant.

        Returns
        -------
        ObjectPrx
            A proxy for ``id``, created by this object adapter.

        Raises
        ------
        AlreadyRegisteredException
            If a servant with the same identity is already registered.
        """
        return self._impl.add(servant, id)

    def addFacet(self, servant: Object, id: Identity, facet: str = "") -> ObjectPrx:
        """
        Adds a servant to this object adapter's Active Servant Map (ASM), while specifying a facet.
        The ASM is a map {identity, facet} -> servant.

        Parameters
        ----------
        servant : Object
            The servant to add.
        id : Identity
            The identity of the Ice object that is implemented by the servant.
        facet : str, optional
            The facet of the Ice object that is implemented by the servant.

        Returns
        -------
        ObjectPrx
            A proxy for ``id`` and ``facet``, created by this object adapter.

        Raises
        ------
        AlreadyRegisteredException
            If a servant with the same identity and facet is already registered.
        """
        return self._impl.addFacet(servant, id, facet)

    def addWithUUID(self, servant: Object) -> ObjectPrx:
        """
        Adds a servant to this object adapter's Active Servant Map (ASM),
        using an automatically generated UUID as its identity.

        Parameters
        ----------
        servant : Object
            The servant to add.

        Returns
        -------
        ObjectPrx
            A proxy with the generated UUID identity created by this object adapter.
        """
        return self._impl.addWithUUID(servant)

    def addFacetWithUUID(self, servant: Object, facet: str) -> ObjectPrx:
        """
        Adds a servant to this object adapter's Active Servant Map (ASM),
        using an automatically generated UUID as its identity. Also specifies a facet.

        Parameters
        ----------
        servant : Object
            The servant to add.
        facet : str
            The facet of the Ice object that is implemented by the servant.

        Returns
        -------
        ObjectPrx
            A proxy with the generated UUID identity and the specified facet.
        """
        return self._impl.addFacetWithUUID(servant, facet)

    def addDefaultServant(self, servant: Object, category: str) -> None:
        """
        Adds a default servant to handle requests for a specific category. When an object adapter dispatches an
        incoming request, it tries to find a servant for the identity and facet carried by the request in the
        following order:
         - The object adapter tries to find a servant for the identity and facet in the Active Servant Map.
         - If this fails, the object adapter tries to find a default servant for the category component of the
           identity.
         - If this fails, the object adapter tries to find a default servant for the empty category, regardless of
           the category contained in the identity.
         - If this fails, the object adapter tries to find a servant locator for the category component of the
           identity. If there is no such servant locator, the object adapter tries to find a servant locator for the
           empty category.
           - If a servant locator is found, the object adapter tries to find a servant using this servant locator.
         - If all the previous steps fail, the object adapter gives up and the caller receives an
           :class:`ObjectNotExistException` or a :class:`FacetNotExistException`.

        Parameters
        ----------
        servant : Object
            The default servant to add.
        category : str
            The category for which the default servant is registered.
            The empty category means it handles all categories.

        Raises
        ------
        AlreadyRegisteredException
            If a default servant with the same category is already registered.
        """
        self._impl.addDefaultServant(servant, category)

    def remove(self, id: Identity) -> Object:
        """
        Removes a servant from the object adapter's Active Servant Map.

        Parameters
        ----------
        id : Identity
            The identity of the Ice object that is implemented by the servant.

        Returns
        -------
        Object
            The removed servant.

        Raises
        ------
        NotRegisteredException
            If no servant with the given identity is registered.
        """
        return self._impl.remove(id)

    def removeFacet(self, id: Identity, facet: str) -> Object:
        """
        Removes a servant from the object adapter's Active Servant Map, while specifying a facet.

        Parameters
        ----------
        id : Identity
            The identity of the Ice object that is implemented by the servant.
        facet : str
            The facet. An empty facet means the default facet.

        Returns
        -------
        Object
            The removed servant.

        Raises
        ------
        NotRegisteredException
            If no servant with the given identity and facet is registered.
        """
        return self._impl.removeFacet(id, facet)

    def removeAllFacets(self, id: Identity) -> dict[str, Object]:
        """
        Removes all facets with the given identity from the Active Servant Map. This function completely removes the
        Ice object, including its default facet.

        Parameters
        ----------
        id : Identity
            The identity of the Ice object to be removed.

        Returns
        -------
        dict[str, Object]
            A collection containing all the facet names and servants of the removed Ice object.

        Raises
        ------
        NotRegisteredException
            If no servant with the given identity is registered.
        """
        return self._impl.removeAllFacets(id)

    def removeDefaultServant(self, category: str) -> Object:
        """
        Removes the default servant for a specific category.

        Parameters
        ----------
        category : str
            The category of the default servant to remove.

        Returns
        -------
        Object
            The default servant.

        Raises
        ------
        NotRegisteredException
            If no default servant is registered for the given category.
        """
        return self._impl.removeDefaultServant(category)

    def find(self, id: Identity) -> Object | None:
        """
        Looks up a servant.

        Notes
        -----
        This function only tries to find the servant in the ASM and among the default servants.
        It does not attempt to locate a servant using servant locators.

        Parameters
        ----------
        id : Identity
            The identity of an Ice object.

        Returns
        -------
        Object | None
            The servant that implements the Ice object with the given identity,
            or ``None`` if no such servant has been found.
        """
        return self._impl.find(id)

    def findFacet(self, id: Identity, facet: str) -> Object | None:
        """
        Looks up a servant with an identity and facet.

        Notes
        -----
        This function only tries to find the servant in the ASM and among the default servants.
        It does not attempt to locate a servant using servant locators.

        Parameters
        ----------
        id : Identity
            The identity of an Ice object.
        facet : str
            The facet of an Ice object. An empty facet means the default facet.

        Returns
        -------
        Object | None
            The servant that implements the Ice object with the given identity and facet,
            or ``None`` if no such servant has been found.
        """
        return self._impl.findFacet(id, facet)

    def findAllFacets(self, id: Identity) -> dict[str, Object]:
        """
        Finds all facets for a given identity in the Active Servant Map.

        Parameters
        ----------
        id : Identity
            The identity.

        Returns
        -------
        dict[str, Object]
            A collection containing all the facet names and servants that have been found. Can be empty.
        """
        return self._impl.findAllFacets(id)

    def findByProxy(self, proxy: ObjectPrx) -> Object | None:
        """
        Looks up a servant with an identity and a facet. It's equivalent to calling :func:`findFacet`.

        Notes
        -----
        This function only tries to find the servant in the ASM and among the default servants.
        It does not attempt to locate a servant using servant locators.

        Parameters
        ----------
        proxy : ObjectPrx
            The proxy that provides the identity and facet to search.

        Returns
        -------
        Object | None
            The servant that matches the identity and facet carried by ``proxy``,
            or ``None`` if no such servant has been found.
        """
        return self._impl.findByProxy(proxy)

    def addServantLocator(self, locator: ServantLocator, category: str) -> None:
        """
        Adds a ServantLocator to this object adapter for a specific category.

        Parameters
        ----------
        locator : ServantLocator
            The servant locator to add.
        category : str
            The category. The empty category means the locator handles all categories.

        Raises
        ------
        AlreadyRegisteredException
            If a servant locator with the same category is already registered.
        """
        self._impl.addServantLocator(locator, category)

    def removeServantLocator(self, category: str) -> ServantLocator:
        """
        Removes a ServantLocator from this object adapter.

        Parameters
        ----------
        category : str
            The category.

        Returns
        -------
        ServantLocator
            The servant locator.

        Raises
        ------
        NotRegisteredException
            If no servant locator with the given category is registered.
        """
        return self._impl.removeServantLocator(category)

    def findServantLocator(self, category: str) -> ServantLocator | None:
        """
        Finds a ServantLocator registered with this object adapter.

        Parameters
        ----------
        category : str
            The category.

        Returns
        -------
        ServantLocator | None
            The servant locator, or ``None`` if not found.
        """
        return self._impl.findServantLocator(category)

    def findDefaultServant(self, category: str) -> Object | None:
        """
        Finds the default servant for a specific category.

        Parameters
        ----------
        category : str
            The category.

        Returns
        -------
        Object | None
            The default servant, or ``None`` if not found.
        """
        return self._impl.findDefaultServant(category)

    def createProxy(self, id: Identity) -> ObjectPrx:
        """
        Creates a proxy from an Ice identity. If this object adapter is configured with an adapter ID, the proxy
        is an indirect proxy that refers to this adapter ID. If a replica group ID is also defined, the proxy is an
        indirect proxy that refers to this replica group ID. Otherwise, the proxy is a direct proxy containing this
        object adapter's published endpoints.

        Parameters
        ----------
        id : Identity
            An Ice identity.

        Returns
        -------
        ObjectPrx
            A proxy with the given identity.
        """
        return self._impl.createProxy(id)

    def createDirectProxy(self, id: Identity) -> ObjectPrx:
        """
        Creates a direct proxy from an Ice identity.

        Parameters
        ----------
        id : Identity
            An Ice identity.

        Returns
        -------
        ObjectPrx
            A proxy with the given identity and the published endpoints of this object adapter.
        """
        return self._impl.createDirectProxy(id)

    def createIndirectProxy(self, id: Identity) -> ObjectPrx:
        """
        Creates an indirect proxy for an Ice identity.

        Parameters
        ----------
        id : Identity
            An Ice identity.

        Returns
        -------
        ObjectPrx
            An indirect proxy with the given identity. If this object adapter is not configured with an adapter
            ID or a replica group ID, the new proxy is a well-known proxy (i.e., an identity-only proxy).
        """
        return self._impl.createIndirectProxy(id)

    def setLocator(self, locator: LocatorPrx) -> None:
        """
        Sets an Ice locator on this object adapter.

        Parameters
        ----------
        locator : LocatorPrx
            The locator used by this object adapter.
        """
        self._impl.setLocator(locator)

    def getLocator(self) -> LocatorPrx | None:
        """
        Gets the Ice locator used by this object adapter.

        Returns
        -------
        LocatorPrx | None
            The locator used by this object adapter, or ``None`` if no locator is used by this object adapter.
        """
        return self._impl.getLocator()

    def getEndpoints(self) -> tuple[Endpoint, ...]:
        """
        Gets the set of endpoints configured on this object adapter.

        Notes
        -----
        This function remains usable after the object adapter has been deactivated.

        Returns
        -------
        tuple[Ice.Endpoint, ...]
            The set of endpoints.
        """
        return self._impl.getEndpoints()

    def getPublishedEndpoints(self) -> tuple[Endpoint, ...]:
        """
        Gets the set of endpoints that proxies created by this object adapter will contain.

        Notes
        -----
        This function remains usable after the object adapter has been deactivated.

        Returns
        -------
        tuple[Ice.Endpoint, ...]
            The set of published endpoints.
        """
        return self._impl.getPublishedEndpoints()

    def setPublishedEndpoints(self, newEndpoints: tuple[Endpoint, ...] | list[Endpoint]) -> None:
        """
        Sets the endpoints that proxies created by this object adapter will contain.

        Parameters
        ----------
        newEndpoints : tuple[Ice.Endpoint, ...] | list[Ice.Endpoint]
            The new set of endpoints that the object adapter will embed in proxies.

        Raises
        ------
        RuntimeError
            If ``newEndpoints`` is empty or if this adapter is associated with a router.
        """
        self._impl.setPublishedEndpoints(newEndpoints)


__all__ = ["ObjectAdapter"]
