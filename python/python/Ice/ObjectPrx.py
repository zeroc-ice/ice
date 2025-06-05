
import IcePy
from .Object import Object

def uncheckedCast(type, proxy, facet=None):
    """
    Downcasts a proxy without confirming the target object's type via a remote invocation.

    Parameters
    ----------
    type : type
        The proxy target type.
    proxy : ObjectPrx
        The source proxy (can be None).

    facet : str, optional
        A facet name.

    Returns
    -------
    ObjectPrx or None
        A proxy with the requested type.
    """
    if proxy is None:
        return None
    if facet is not None:
        proxy = proxy.ice_facet(facet)
    return IcePy.ObjectPrx.newProxy(type, proxy)

def checkedCast(type, proxy, facet=None, context=None):
    """
    Downcasts a proxy after confirming the target object's type via a remote invocation.

    Parameters
    ----------
    type : type
        The proxy target type.
    proxy : ObjectPrx
        The source proxy (can be None).

    facet : str, optional
        A facet name.

    context : dict[str, str], optional
        The request context.

    Returns
    -------
    ObjectPrx or None
        A proxy with the requested type, or None if the target object does not support the requested type.
    """
    if proxy is None:
        return None
    if facet is not None:
        proxy = proxy.ice_facet(facet)
    return IcePy.ObjectPrx.newProxy(type, proxy) if proxy.ice_isA(type.ice_staticId(), context=context) else None

async def checkedCastAsync(type, proxy, facet=None, context=None):
    """
    Downcasts a proxy after confirming the target object's type via a remote invocation.

    Parameters
    ----------
    type : type
        The proxy target type.
    proxy : ObjectPrx
        The source proxy (can be None).

    facet : str, optional
        A facet name.

    context : dict[str, str], optional
        The request context.

    Returns
    -------
    ObjectPrx or None
        A proxy with the requested type, or None if the target object does not support the requested type.
    """
    if proxy is None:
        return None
    if facet is not None:
        proxy = proxy.ice_facet(facet)
    b = await proxy.ice_isAAsync(type.ice_staticId(), context=context)
    return IcePy.ObjectPrx.newProxy(type, proxy) if b else None

class ObjectPrx(IcePy.ObjectPrx):
    """
    The base class for all proxies.
    """

    @staticmethod
    def uncheckedCast(proxy, facet=None):
        """
        Downcasts a proxy without confirming the target object's type via a remote invocation.

        Parameters
        ----------
        proxy : ObjectPrx
            The source proxy (can be None).

        facet : str, optional
            A facet name.

        Returns
        -------
        ObjectPrx or None
            A proxy with the requested type, or None if the target object does not support the requested type.
        """
        return uncheckedCast(ObjectPrx, proxy, facet)

    @staticmethod
    def checkedCast(proxy, facet=None, context=None):
        """
        Downcasts a proxy after confirming the target object's type via a remote invocation.

        Parameters
        ----------
        proxy : ObjectPrx
            The source proxy (can be None).

        facet : str, optional
            A facet name.

        context : dict[str, str], optional
            The request context.

        Returns
        -------
        ObjectPrx or None
            A proxy with the requested type, or None if the target object does not support the requested type.
        """
        return checkedCast(ObjectPrx, proxy, facet, context)

    @staticmethod
    def checkedCastAsync(proxy, facet=None, context=None):
        """
        Downcasts a proxy after confirming the target object's type via a remote invocation.

        Parameters
        ----------
        proxy : ObjectPrx
            The source proxy (can be None).

        facet : str, optional
            A facet name.

        context : dict[str, str], optional
            The request context.

        Returns
        -------
        ObjectPrx or None
            A proxy with the requested type, or None if the target object does not support the requested type.
        """
        return checkedCastAsync(ObjectPrx, proxy, facet, context)

    @staticmethod
    def ice_staticId():
        """
        Gets the Slice type ID of the interface associated with this proxy.

        Returns
        -------
        str
            The type ID, "::Ice::Object".
        """
        return "::Ice::Object"

    def ice_getCommunicator(self):
        """
        Return the communicator that created this proxy.

        Returns
        -------
        Communicator
            The communicator that created this proxy.
        """
        return super().ice_getCommunicator()

    def ice_isA(self, id, context=None):
        """
        Test whether this object supports a specific Slice interface.

        Parameters
        ----------
        id : str
            The type ID of the Slice interface to test against.
        context : dict[str, str], optional
            The context dictionary for the invocation.

        Returns
        -------
        bool
            True if the target object has the interface specified by id or derives from the interface specified by id.
        """
        return Object._op_ice_isA.invoke(self, ((id,), context))

    def ice_isAAsync(self, id, context=None):
        """
        Test whether this object supports a specific Slice interface.

        Parameters
        ----------
        id : str
            The type ID of the Slice interface to test against.
        context : dict[str, str], optional
            The context dictionary for the invocation.

        Returns
        -------
        bool
            True if the target object has the interface specified by id or derives from the interface specified by id.
        """
        return Object._op_ice_isA.invokeAsync(self, ((id,), context))

    def ice_ping(self, context=None):
        """
        Test whether the target object of this proxy can be reached.

        Parameters
        ----------
        context : dict[str, str], optional
            The context dictionary for the invocation.

        Examples
        --------
        >>> obj.ice_ping(context={'key': 'value'})
        """
        Object._op_ice_ping.invoke(self, ((), context))

    def ice_pingAsync(self, context=None):
        """
        Test whether the target object of this proxy can be reached.

        Parameters
        ----------
        context : dict[str, str], optional
            The context dictionary for the invocation.

        Examples
        --------
        >>> obj.ice_ping(context={'key': 'value'})
        """
        return Object._op_ice_ping.invokeAsync(self, ((), context))

    def ice_ids(self, context=None):
        """
        Return the Slice type IDs of the interfaces supported by the target object of this proxy.

        Parameters
        ----------
        context : dict[str, str], optional
            The context dictionary for the invocation.

        Returns
        -------
        list of str
            The Slice type IDs of the interfaces supported by the target object, in alphabetical order.
        """
        return Object._op_ice_ids.invoke(self, ((), context))

    def ice_idsAsync(self, context=None):
        """
        Return the Slice type IDs of the interfaces supported by the target object of this proxy.

        Parameters
        ----------
        context : dict[str, str], optional
            The context dictionary for the invocation.

        Returns
        -------
        list of str
            The Slice type IDs of the interfaces supported by the target object, in alphabetical order.
        """
        return Object._op_ice_ids.invokeAsync(self, ((), context))

    def ice_id(self, context=None):
        """
        Return the Slice type ID of the most-derived interface supported by the target object of this proxy.

        Parameters
        ----------
        context : dict[str, str], optional
            The context dictionary for the invocation.

        Returns
        -------
        str
            The Slice type ID of the most-derived interface.
        """
        return Object._op_ice_id.invoke(self, ((), context))

    def ice_idAsync(self, context=None):
        """
        Return the Slice type ID of the most-derived interface supported by the target object of this proxy.

        Parameters
        ----------
        context : dict[str, str], optional
            The context dictionary for the invocation.

        Returns
        -------
        str
            The Slice type ID of the most-derived interface.
        """
        return Object._op_ice_id.invokeAsync(self, ((), context))

    def ice_getIdentity(self):
        """
        Return the identity embedded in this proxy.

        Returns
        -------
        Ice.Identity
            The identity of the target object.
        """
        return super().ice_getIdentity()

    def ice_identity(self, newIdentity):
        """
        Create a new proxy that is identical to this proxy, except for the per-proxy context.

        Parameters
        ----------
        newIdentity : Identity
            The identity for the new proxy.

        Returns
        -------
        ObjectPrx
            The proxy with the new identity.
        """
        return super().ice_identity(newIdentity)

    def ice_getContext(self):
        """
        Returns the per-proxy context for this proxy.

        Returns
        -------
        dict or None
            The per-proxy context. If the proxy does not have a per-proxy (implicit) context, the return value is None.
        """
        return super().ice_getContext()

    def ice_context(self, new_context):
        """
        Creates a new proxy that is identical to this proxy, except for the per-proxy context.

        Parameters
        ----------
        new_context : dict[str, str]
            The context for the new proxy.

        Returns
        -------
        ObjectPrx
            The proxy with the new per-proxy context.
        """
        return super().ice_context(new_context)

    def ice_getFacet(self):
        """
        Returns the facet for this proxy.

        Returns
        -------
        str
            The facet for this proxy. If the proxy uses the default facet, the return value is the empty string.
        """
        return super().ice_getFacet()

    def ice_facet(self, new_facet):
        """
        Creates a new proxy that is identical to this proxy, except for the facet.

        Parameters
        ----------
        new_facet : str
            The facet for the new proxy.

        Returns
        -------
        ObjectPrx
            The proxy with the new facet.
        """
        return super().ice_facet(new_facet)

    def ice_getAdapterId(self):
        """
        Returns the adapter ID for this proxy.

        Returns
        -------
        str
            The adapter ID. If the proxy does not have an adapter ID, the return value is the empty string.
        """
        return super().ice_getAdapterId()

    def ice_adapterId(self, newAdapterId):
        """
        Creates a new proxy that is identical to this proxy, except for the adapter ID.

        Parameters
        ----------
        newAdapterId : str
            The adapter ID for the new proxy.

        Returns
        -------
        ObjectPrx
            The proxy with the new adapter ID.
        """
        return super().ice_adapterId(newAdapterId)

    def ice_getEndpoints(self):
        """
        Returns the endpoints used by this proxy.

        Returns
        -------
        list of Ice.Endpoint
            The endpoints used by this proxy.
        """
        return super().ice_getEndpoints()

    def ice_endpoints(self, newEndpoints):
        """
        Creates a new proxy that is identical to this proxy, except for the endpoints.

        Parameters
        ----------
        newEndpoints : list of Ice.Endpoint
            The endpoints for the new proxy.

        Returns
        -------
        ObjectPrx
            The proxy with the new endpoints.
        """
        return super().ice_endpoints(newEndpoints)

    def ice_getLocatorCacheTimeout(self):
        """
        Returns the locator cache timeout of this proxy.

        Returns
        -------
        int
            The locator cache timeout value (in seconds).
        """
        return super().ice_getLocatorCacheTimeout()

    def ice_locatorCacheTimeout(self, timeout):
        """
        Creates a new proxy that is identical to this proxy, except for the locator cache timeout.

        Parameters
        ----------
        timeout : int
            The new locator cache timeout (in seconds).

        Returns
        -------
        ObjectPrx
            The proxy with the new locator cache timeout.
        """
        return super().ice_locatorCacheTimeout(timeout)

    def ice_invocationTimeout(self, timeout):
        """
        Creates a new proxy that is identical to this proxy, except for the invocation timeout.

        Parameters
        ----------
        timeout : int
            The new invocation timeout (in seconds).

        Returns
        -------
        ObjectPrx
            The proxy with the new invocation timeout.
        """
        return super().ice_invocationTimeout(timeout)

    def ice_getInvocationTimeout(self):
        """
        Returns the invocation timeout of this proxy.

        Returns
        -------
        int
            The invocation timeout value (in seconds).
        """
        return super().ice_getInvocationTimeout()

    def ice_isConnectionCached(self):
        """
        Returns whether this proxy caches connections.

        Returns
        -------
        bool
            True if this proxy caches connections; False otherwise.
        """
        return super().ice_isConnectionCached()

    def ice_connectionCached(self, newCache):
        """
        Creates a new proxy that is identical to this proxy, except for connection caching.

        Parameters
        ----------
        newCache : bool
            True if the new proxy should cache connections; False otherwise.

        Returns
        -------
        ObjectPrx
            The new proxy with the specified caching policy.
        """
        return super().ice_connectionCached(newCache)

    def ice_getEndpointSelection(self):
        """
        Returns how this proxy selects endpoints (randomly or ordered).

        Returns
        -------
        EndpointSelectionType
            The endpoint selection policy.
        """
        return super().ice_getEndpointSelection()

    def ice_endpointSelection(self, newType):
        """
        Creates a new proxy that is identical to this proxy, except for the endpoint selection policy.

        Parameters
        ----------
        newType : EndpointSelectionType
            The new endpoint selection policy.

        Returns
        -------
        ObjectPrx
            The new proxy with the specified endpoint selection policy.
        """
        return super().ice_endpointSelection(newType)

    def ice_isSecure(self):
        """
        Returns whether this proxy communicates only via secure endpoints.

        Returns
        -------
        bool
            True if this proxy communicates only via secure endpoints; False otherwise.
        """
        return super().ice_isSecure()

    def ice_secure(self, secure):
        """
        Creates a new proxy that is identical to this proxy, except for how it selects endpoints.

        Parameters
        ----------
        secure : bool
            If secure is True, only endpoints that use a secure transport are used by the new proxy.
            If secure is False, the returned proxy uses both secure and insecure endpoints.

        Returns
        -------
        ObjectPrx
            The new proxy with the specified selection policy.
        """
        return super().ice_secure(secure)

    def ice_encodingVersion(self, version):
        """
        Creates a new proxy that is identical to this proxy, except for the encoding used to marshal parameters.

        Parameters
        ----------
        version : EncodingVersion
            The encoding version to use to marshal requests parameters.

        Returns
        -------
        ObjectPrx
            The new proxy with the specified encoding version.
        """
        return super().ice_encodingVersion(version)

    def ice_getEncodingVersion(self):
        """
        Returns the encoding version used to marshal requests parameters.

        Returns
        -------
        EncodingVersion
            The encoding version.
        """
        return super().ice_getEncodingVersion()

    def ice_isPreferSecure(self):
        """
        Returns whether this proxy prefers secure endpoints.

        Returns
        -------
        bool
            True if the proxy always attempts to invoke via secure endpoints before it attempts to use insecure endpoints; False otherwise.
        """
        return super().ice_isPreferSecure()

    def ice_preferSecure(self, preferSecure):
        """
        Creates a new proxy that is identical to this proxy, except for its endpoint selection policy.

        Parameters
        ----------
        preferSecure : bool
            If b is True, the new proxy will use secure endpoints for invocations and only use insecure endpoints if an invocation cannot be made via secure endpoints.
            If b is False, the proxy prefers insecure endpoints to secure ones.

        Returns
        -------
        ObjectPrx
            The new proxy with the new endpoint selection policy.
        """
        return super().ice_preferSecure(preferSecure)

    def ice_getRouter(self):
        """
        Returns the router for this proxy.

        Returns
        -------
        RouterPrx or None
            The router for the proxy. If no router is configured for the proxy, the return value is None.
        """
        return super().ice_getRouter()

    def ice_router(self, router):
        """
        Creates a new proxy that is identical to this proxy, except for the router.

        Parameters
        ----------
        router : RouterPrx or None
            The router for the new proxy.

        Returns
        -------
        ObjectPrx
            The new proxy with the specified router.
        """
        return super().ice_router(router)

    def ice_getLocator(self):
        """
        Returns the locator for this proxy.

        Returns
        -------
        LocatorPrx or None
            The locator for this proxy. If no locator is configured, the return value is None.
        """
        return super().ice_getLocator()

    def ice_locator(self, locator):
        """
        Creates a new proxy that is identical to this proxy, except for the locator.

        Parameters
        ----------
        locator : LocatorPrx or None
            The locator for the new proxy.

        Returns
        -------
        ObjectPrx
            The new proxy with the specified locator.
        """
        return super().ice_locator(locator)

    def ice_isCollocationOptimized(self):
        """
        Returns whether this proxy uses collocation optimization.

        Returns
        -------
        bool
            True if the proxy uses collocation optimization; False otherwise.
        """
        return super().ice_isCollocationOptimized()

    def ice_collocationOptimized(self, collocated):
        """
        Creates a new proxy that is identical to this proxy, except for collocation optimization.

        Parameters
        ----------
        collocated : bool
            True if the new proxy enables collocation optimization; False otherwise.

        Returns
        -------
        ObjectPrx
            The new proxy with the specified collocation optimization.
        """
        return super().ice_collocationOptimized(collocated)

    def ice_twoway(self):
        """
        Creates a new proxy that is identical to this proxy, but uses twoway invocations.

        Returns
        -------
        ObjectPrx
            A new proxy that uses twoway invocations.
        """
        return super().ice_twoway()

    def ice_isTwoway(self):
        """
        Returns whether this proxy uses twoway invocations.

        Returns
        -------
        bool
            True if this proxy uses twoway invocations; False otherwise.
        """
        return super().ice_isTwoway()

    def ice_oneway(self):
        """
        Creates a new proxy that is identical to this proxy, but uses oneway invocations.

        Returns
        -------
        ObjectPrx
            A new proxy that uses oneway invocations.
        """
        return super().ice_oneway()

    def ice_isOneway(self):
        """
        Returns whether this proxy uses oneway invocations.

        Returns
        -------
        bool
            True if this proxy uses oneway invocations; False otherwise.
        """
        return super().ice_isOneway()

    def ice_batchOneway(self):
        """
        Creates a new proxy that is identical to this proxy, but uses batch oneway invocations.

        Returns
        -------
        ObjectPrx
            A new proxy that uses batch oneway invocations.
        """
        return super().ice_batchOneway()

    def ice_isBatchOneway(self):
        """
        Returns whether this proxy uses batch oneway invocations.

        Returns
        -------
        bool
            True if this proxy uses batch oneway invocations; False otherwise.
        """
        return super().ice_isBatchOneway()

    def ice_datagram(self):
        """
        Creates a new proxy that is identical to this proxy, but uses datagram invocations.

        Returns
        -------
        ObjectPrx
            A new proxy that uses datagram invocations.
        """
        return super().ice_datagram()

    def ice_isDatagram(self):
        """
        Returns whether this proxy uses datagram invocations.

        Returns
        -------
        bool
            True if this proxy uses datagram invocations; False otherwise.
        """
        return super().ice_isDatagram()

    def ice_batchDatagram(self):
        """
        Creates a new proxy that is identical to this proxy, but uses batch datagram invocations.

        Returns
        -------
        ObjectPrx
            A new proxy that uses batch datagram invocations.
        """
        return super().ice_batchDatagram()

    def ice_isBatchDatagram(self):
        """
        Returns whether this proxy uses batch datagram invocations.

        Returns
        -------
        bool
            True if this proxy uses batch datagram invocations; False otherwise.
        """
        return super().ice_isBatchDatagram()

    def ice_compress(self, compress):
        """
        Creates a new proxy that is identical to this proxy, except for compression.

        Parameters
        ----------
        compress : bool
            True enables compression for the new proxy; False disables compression.

        Returns
        -------
        ObjectPrx
            A new proxy with the specified compression setting.
        """
        return super().ice_compress(compress)

    def ice_getCompress(self):
        """
        Obtains the compression override setting of this proxy.

        Returns
        -------
        bool or None
            The compression override setting. If no optional value is present, no override is set. Otherwise, true if compression is enabled, false otherwise.
        """
        return super().ice_getCompress()

    def ice_connectionId(self, connectionId):
        """
        Creates a new proxy that is identical to this proxy, except for its connection ID.

        Parameters
        ----------
        connectionId : str
            The connection ID for the new proxy. An empty string removes the connection ID.

        Returns
        -------
        ObjectPrx
            A new proxy with the specified connection ID.
        """
        return super().ice_connectionId(connectionId)

    def ice_getConnectionId(self):
        """
        Returns the connection id of this proxy.

        Returns
        -------
        str
            The connection id.
        """
        return super().ice_getConnectionId()

    def ice_fixed(self, connection):
        """
        Returns a proxy that is identical to this proxy, except it's a fixed proxy bound
        to the given connection.

        Parameters
        ----------
        connection : Ice.Connection
            The fixed proxy connection.

        Returns
        -------
        ObjectPrx
            A fixed proxy bound to the given connection.
        """
        return super().ice_fixed(connection)

    def ice_isFixed(self):
        """
        Returns whether this proxy is a fixed proxy.

        Returns
        -------
        bool
            True if this is a fixed proxy; False otherwise.
        """
        return super().ice_isFixed()

    def ice_getConnection(self):
        """
        Returns the Connection for this proxy. If the proxy does not yet have an established connection,
        it first attempts to create a connection.

        Returns
        -------
        Connection
            The Connection for this proxy.
        """
        return super().ice_getConnection()

    def ice_getCachedConnection(self):
        """
        Returns the cached Connection for this proxy. If the proxy does not yet have an established
        connection, it does not attempt to create a connection.

        Returns
        -------
        Connection or None
            The cached Connection for this proxy (None if the proxy does not have an established connection).
        """
        return super().ice_getCachedConnection()

    def ice_flushBatchRequests(self):
        """
        Flushes any pending batched requests for this proxy. The call blocks until the flush is complete.
        """
        return super().ice_flushBatchRequests()

    def __repr__(self):
        return (
            f"{self.__class__.__module__}.{self.__class__.__qualname__}("
            f"communicator={self.ice_getCommunicator()!r}, "
            f"proxyString={self.ice_toString()!r})"
        )

    def __str__(self):
        return repr(self)
