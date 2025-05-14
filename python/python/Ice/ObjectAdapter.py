# Copyright (c) ZeroC, Inc.

from typing import final

@final
class ObjectAdapter:
    """
    The object adapter provides an up-call interface from the Ice runtime to the implementation of Ice objects.

    The object adapter is responsible for receiving requests from endpoints, and for mapping between servants,
    identities, and proxies.
    """

    def __init__(self, impl):
        self._impl = impl

    def getName(self):
        """
        Get the name of this object adapter.

        Returns
        -------
        str
            The name of this object adapter.
        """
        return self._impl.getName()

    def getCommunicator(self):
        """
        Get the communicator this object adapter belongs to.

        Returns
        -------
        Ice.Communicator
            The communicator this object adapter belongs to.
        """
        communicator = self._impl.getCommunicator()
        return communicator._getWrapper()

    def activate(self):
        """
        Activate all endpoints that belong to this object adapter.

        After activation, the object adapter can dispatch requests received through its endpoints.
        """
        self._impl.activate()

    def hold(self):
        """
        Temporarily hold receiving and dispatching requests.

        The object adapter can be reactivated with the `activate` operation. Holding is not immediate;
        i.e., after `hold` returns, the object adapter might still be active for some time.
        You can use `waitForHold` to wait until holding is complete.
        """
        self._impl.hold()

    def waitForHold(self):
        """
        Wait until the object adapter holds requests.

        Calling `hold` initiates holding of requests, and `waitForHold` only returns when holding of requests has been completed.
        """
        #
        # TODO should be part of the documented behavior above. Should we add a timeout parameter with a default value?
        #
        # If invoked by the main thread, waitForHold only blocks for
        # the specified timeout in order to give us a chance to handle
        # signals.
        #
        while not self._impl.waitForHold(1000):
            pass

    def deactivate(self):
        """
        Deactivates this object adapter: stop accepting new connections from clients and close gracefully all incoming
        connections created by this object adapter once all outstanding dispatches have completed.

        If this object adapter is indirect, this method also unregisters the object adapter from the Locator.
        This method does not cancel outstanding dispatches--it lets them execute until completion. A new incoming
        request on an existing connection will be accepted and can delay the closure of the connection.
        A deactivated object adapter cannot be reactivated again; it can only be destroyed.
        """
        self._impl.deactivate()

    def waitForDeactivate(self):
        """
        Wait until `deactivate` is called on this object adapter and all connections accepted by this object adapter are
        closed.

        A connection is closed only after all outstanding dispatches on this connection have completed.
        """
        #
        # TODO should be part of the documented behavior above. Should we add a timeout parameter with a default value?
        #
        # If invoked by the main thread, waitForDeactivate only blocks for
        # the specified timeout in order to give us a chance to handle
        # signals.
        #
        while not self._impl.waitForDeactivate(1000):
            pass

    def isDeactivated(self):
        """
        Checks if this object adapter has been deactivated.

        Returns
        -------
        bool
            True if `deactivate` has been called on this object adapter; otherwise, False.
        """
        self._impl.isDeactivated()

    def destroy(self):
        """
        Destroys this object adapter and cleans up all resources held by this object adapter.

        Once this method has returned, it is possible to create another object adapter with the same name.
        """
        self._impl.destroy()

    def add(self, servant, id):
        """
        Add a servant to this object adapter's Active Servant Map.

        Note that one servant can implement several Ice objects by registering the servant with multiple identities.
        Adding a servant with an identity that is already in the map throws `AlreadyRegisteredException`.

        Parameters
        ----------
        servant : Ice.Object
            The servant to add.
        id : Ice.Identity
            The identity of the Ice object that is implemented by the servant.

        Returns
        -------
        Ice.ObjectPrx
            A proxy that matches the given identity and this object adapter.
        """
        return self._impl.add(servant, id)

    def addFacet(self, servant, id, facet):
        """
        Add a servant with a facet to this object adapter's Active Servant Map.

        Calling `add(servant, id)` is equivalent to calling `addFacet` with an empty facet.

        Parameters
        ----------
        servant : Ice.Object
            The servant to add.
        id : Ice.Identity
            The identity of the Ice object that is implemented by the servant.
        facet : str
            The facet. An empty facet means the default facet.

        Returns
        -------
        Ice.ObjectPrx
            A proxy that matches the given identity, facet, and this object adapter.
        """
        return self._impl.addFacet(servant, id, facet)

    def addWithUUID(self, servant):
        """
        Add a servant to this object adapter's Active Servant Map, using an automatically generated UUID as its identity.

        Note that the generated UUID identity can be accessed using the proxy's `ice_getIdentity` operation.

        Parameters
        ----------
        servant : Ice.Object
            The servant to add.

        Returns
        -------
        Ice.ObjectPrx
            A proxy that matches the generated UUID identity and this object adapter.
        """
        return self._impl.addWithUUID(servant)

    def addFacetWithUUID(self, servant, facet):
        """
        Add a servant with a facet to this object adapter's Active Servant Map, using an automatically generated UUID as its identity.

        Calling `addWithUUID(servant)` is equivalent to calling `addFacetWithUUID` with an empty facet.

        Parameters
        ----------
        servant : Ice.Object
            The servant to add.
        facet : str
            The facet. An empty facet means the default facet.

        Returns
        -------
        Ice.ObjectPrx
            A proxy that matches the generated UUID identity, facet, and this object adapter.
        """
        return self._impl.addFacetWIthUUID(servant, facet)

    def addDefaultServant(self, servant, category):
        """
        Add a default servant to handle requests for a specific category.

        Adding a default servant for a category for which a default servant is already registered throws
        `AlreadyRegisteredException`. To dispatch operation calls on servants, the object adapter tries to
        find a servant for a given Ice object identity and facet in the following order:

        1. The object adapter tries to find a servant for the identity and facet in the Active Servant Map.
        2. If no servant has been found in the Active Servant Map, the object adapter tries to find a default servant
           for the category component of the identity.
        3. If no servant has been found by any of the preceding steps, the object adapter tries to find a default servant
           for an empty category, regardless of the category contained in the identity.
        4. If no servant has been found by any of the preceding steps, the object adapter gives up and the caller
           receives `ObjectNotExistException` or `FacetNotExistException`.


        Parameters
        ----------
        servant : Ice.Object
            The default servant.
        category : str
            The category for which the default servant is registered. An empty category means it will handle all categories.
        """
        self._impl.addDefaultServant(servant, category)

    def remove(self, id):
        """
        Remove a servant (that is, the default facet) from the object adapter's Active Servant Map.

        Removing an identity that is not in the map throws `NotRegisteredException`.

        Parameters
        ----------
        id : Ice.Identity
            The identity of the Ice object that is implemented by the servant. If the servant implements multiple Ice
            objects, `remove` has to be called for all those Ice objects.

        Returns
        -------
        Ice.Object
            The removed servant.
        """
        return self._impl.remove(id)

    def removeFacet(self, id, facet):
        """
        Remove a servant with a facet from the object adapter's Active Servant Map.

        Calling `remove(id)` is equivalent to calling `removeFacet` with an empty facet.

        Parameters
        ----------
        id : Ice.Identity
            The identity of the Ice object that is implemented by the servant.
        facet : str
            The facet. An empty facet means the default facet.

        Returns
        -------
        Ice.Object
            The removed servant.
        """
        return self._impl.removeFacet(id, facet)

    def removeAllFacets(self, id):
        """
        Remove all facets with the given identity from the Active Servant Map.

        The operation completely removes the Ice object, including its default facet. Removing an identity that is not
        in the map throws `NotRegisteredException`.

        Parameters
        ----------
        id : Ice.Identity
            The identity of the Ice object to be removed.

        Returns
        -------
        dict[str, Ice.Object]
            A collection containing all the facet names and servants of the removed Ice object.
        """
        return self._impl.removeAllFacets(id)

    def removeDefaultServant(self, category):
        """
        Remove the default servant for a specific category.

        Attempting to remove a default servant for a category that is not registered throws `NotRegisteredException`.

        Parameters
        ----------
        category : str
            The category of the default servant to remove.

        Returns
        -------
        Ice.Object
            The default servant.
        """
        return self._impl.removeDefaultServant(category)

    def find(self, id):
        """
        Look up a servant in this object adapter's Active Servant Map by the identity of the Ice object it implements.

        This operation only tries to look up a servant in the Active Servant Map. It does not attempt to find a servant
        by using any installed ServantLocator.

        Parameters
        ----------
        id : Ice.Identity
            The identity of the Ice object for which the servant should be returned.

        Returns
        -------
        Ice.Object or None
            The servant that implements the Ice object with the given identity, or None if no such servant has been found.
        """
        return self._impl.find(id)

    def findFacet(self, id, facet):
        """
        Look up a servant in this object adapter's Active Servant Map by the identity and facet of the Ice object it implements.

        Calling `find(id)` is equivalent to calling `findFacet` with an empty facet.

        Parameters
        ----------
        id : Ice.Identity
            The identity of the Ice object for which the servant should be returned.
        facet : str
            The facet. An empty facet means the default facet.

        Returns
        -------
        Ice.Object or None
            The servant that implements the Ice object with the given identity and facet, or None if no such servant has been found.
        """
        return self._impl.findFacet(id, facet)

    def findAllFacets(self, id):
        """
        Find all facets with the given identity in the Active Servant Map.

        Parameters
        ----------
        id : Ice.Identity
            The identity of the Ice object for which the facets should be returned.

        Returns
        -------
        dict[str, Ice.Object]
            A dictionary containing all the facet names and servants that have been found, or an empty dictionary if
            there is no facet for the given identity.
        """
        return self._impl.findAllFacets(id)

    def findByProxy(self, proxy):
        """
        Look up a servant in this object adapter's Active Servant Map, given a proxy.

        This operation only tries to look up a servant in the Active Servant Map. It does not attempt to find a servant
        by using any installed ServantLocator.

        Parameters
        ----------
        proxy : Ice.ObjectPrx
            The proxy for which the servant should be returned.

        Returns
        -------
        Ice.Object or None
            The servant that matches the proxy, or None if no such servant has been found.
        """
        return self._impl.findByProxy(proxy)

    def addServantLocator(self, locator, category):
        """
        Add a Servant Locator to this object adapter.

        Adding a servant locator for a category for which a servant locator is already registered throws
        `AlreadyRegisteredException`. To dispatch operation calls on servants, the object adapter tries to
        find a servant for a given Ice object identity and facet in the following order:

        1. The object adapter tries to find a servant for the identity and facet in the Active Servant Map.
        2. If no servant has been found in the Active Servant Map, the object adapter tries to find a servant locator
           for the category component of the identity. If a locator is found, the object adapter tries to find a servant
           using this locator.
        3. If no servant has been found by any of the preceding steps, the object adapter tries to find a locator for
           an empty category, regardless of the category contained in the identity. If a locator is found, the object
           adapter tries to find a servant using this locator.
        4. If no servant has been found by any of the preceding steps, the object adapter gives up and the caller
           receives `ObjectNotExistException` or `FacetNotExistException`.

        Only one locator for the empty category can be installed.

        Parameters
        ----------
        locator : Ice.ServantLocator
            The locator to add.
        category : str
            The category for which the Servant Locator can locate servants, or an empty string if the Servant Locator
            does not belong to any specific category.
        """
        self._impl.addServantLocator(locator, category)

    def removeServantLocator(self, category):
        """
        Remove a Servant Locator from this object adapter.

        Parameters
        ----------
        category : str
            The category for which the Servant Locator can locate servants, or an empty string if the Servant Locator
            does not belong to any specific category.

        Returns
        -------
        Ice.ServantLocator
            The Servant Locator.

        Raises
        ------
        NotRegisteredException
            If no Servant Locator was found for the given category.
        """
        return self._impl.removeServantLocator(category)

    def findServantLocator(self, category):
        """
        Find a Servant Locator installed with this object adapter.

        Parameters
        ----------
        category : str
            The category for which the Servant Locator can locate servants, or an empty string if the Servant Locator
            does not belong to any specific category.

        Returns
        -------
        Ice.ServantLocator or None
            The Servant Locator, or None if no Servant Locator was found for the given category.
        """
        return self._impl.findServantLocator(category)

    def findDefaultServant(self, category):
        """
        Find the default servant for a specific category.

        Parameters
        ----------
        category : str
            The category of the default servant to find.

        Returns
        -------
        Ice.Object or None
            The default servant, or None if no default servant was registered for the category.
        """
        return self._impl.findDefaultServant(category)

    def createProxy(self, id):
        """
        Create a proxy for the object with the given identity.

        If this object adapter is configured with an adapter ID, the return value is an indirect proxy that refers to the
        adapter ID. If a replica group ID is also defined, the return value is an indirect proxy that refers to the replica
        group ID. Otherwise, if no adapter ID is defined, the return value is a direct proxy containing this object adapter's
        published endpoints.

        Parameters
        ----------
        id : Ice.Identity
            The object's identity.

        Returns
        -------
        Ice.ObjectPrx
            A proxy for the object with the given identity.
        """
        return self._impl.createProxy(id)

    def createDirectProxy(self, id):
        """
        Create a direct proxy for the object with the given identity.

        The returned proxy contains this object adapter's published endpoints.

        Parameters
        ----------
        id : Ice.Identity
            The object's identity.

        Returns
        -------
        Ice.ObjectPrx
            A proxy for the object with the given identity.
        """
        return self._impl.createDirectProxy(id)

    def createIndirectProxy(self, id):
        """
        Create an indirect proxy for the object with the given identity.

        If this object adapter is configured with an adapter ID, the return value refers to the adapter ID. Otherwise,
        the return value contains only the object identity.

        Parameters
        ----------
        id : Ice.Identity
            The object's identity.

        Returns
        -------
        Ice.ObjectPrx
            A proxy for the object with the given identity.
        """
        return self._impl.createIndirectProxy(id)

    def setLocator(self, locator):
        """
        Set an Ice locator for this object adapter.

        By doing so, the object adapter will register itself with the locator registry when it is activated for the first
        time. Furthermore, the proxies created by this object adapter will contain the adapter identifier instead of its
        endpoints. The adapter identifier must be configured using the AdapterId property.

        Parameters
        ----------
        locator : Ice.LocatorPrx
            The locator used by this object adapter.
        """
        self._impl.setLocator(locator)

    def getLocator(self):
        """
        Get the Ice locator used by this object adapter.

        Returns
        -------
        Ice.LocatorPrx or None
            The locator used by this object adapter, or None if no locator is used by this object adapter.
        """
        return self._impl.getLocator()

    def getEndpoints(self):
        """
        Get the set of endpoints configured with this object adapter.

        Returns
        -------
        tuple of Ice.Endpoint
            The set of endpoints.
        """
        return self._impl.getEndpoints()

    def getPublishedEndpoints(self):
        """
        Get the set of endpoints that proxies created by this object adapter will contain.

        Returns
        -------
        tuple of Ice.Endpoint
            The set of published endpoints.
        """
        return self._impl.getPublishedEndpoints()

    def setPublishedEndpoints(self, newEndpoints):
        """
        Set the endpoints that proxies created by this object adapter will contain.

        Parameters
        ----------
        newEndpoints : tuple of Ice.Endpoint
            The new set of endpoints that the object adapter will embed in proxies.
        """
        self._impl.setPublishedEndpoints(newEndpoints)
