# Copyright (c) ZeroC, Inc.

# Avoid evaluating annotations at function definition time.
from __future__ import annotations

from collections.abc import Awaitable
from typing import TYPE_CHECKING, Self, overload

import IcePy

from .Object import Object

if TYPE_CHECKING:
    from typing import Type, TypeVar

    from .Communicator import Communicator
    from .EncodingVersion import EncodingVersion
    from .EndpointSelectionType import EndpointSelectionType
    from .IcePyTypes import Connection, Endpoint
    from .Identity import Identity
    from .Locator import LocatorPrx
    from .Router import RouterPrx

    T = TypeVar("T", bound=ObjectPrx)


@overload
def uncheckedCast(type: Type[T], proxy: ObjectPrx, facet: str | None = None) -> T: ...


@overload
def uncheckedCast(type: Type[T], proxy: None, facet: str | None = None) -> None: ...


def uncheckedCast(type: Type[T], proxy: ObjectPrx | None, facet: str | None = None) -> T | None:
    """
    Creates a new proxy from an existing proxy.

    Parameters
    ----------
    type : Type[T]
        The proxy target type.
    proxy : ObjectPrx | None
        The source proxy.
    facet : str | None, optional
        A facet name.

    Returns
    -------
    T | None
        A new proxy with the requested type and facet, or ``None`` if the source proxy is ``None``.
    """
    if proxy is None:
        return None
    if facet is not None:
        proxy = proxy.ice_facet(facet)
    return IcePy.ObjectPrx.newProxy(type, proxy)


def checkedCast(
    type: Type[T], proxy: ObjectPrx | None, facet: str | None = None, context: dict[str, str] | None = None
) -> T | None:
    """
    Creates a new proxy from an existing proxy after confirming the target object's type via a remote invocation.

    Parameters
    ----------
    type : Type[T]
        The proxy target type.
    proxy : ObjectPrx | None
        The source proxy.
    facet : str | None, optional
        A facet name.
    context : dict[str, str] | None, optional
        The request context.

    Returns
    -------
    T | None
        A new proxy with the requested type and facet, or ``None`` if the source proxy is ``None`` or if the target
        object/facet does not support the requested type.
    """
    if proxy is None:
        return None
    if facet is not None:
        proxy = proxy.ice_facet(facet)
    return IcePy.ObjectPrx.newProxy(type, proxy) if proxy.ice_isA(type.ice_staticId(), context=context) else None


async def checkedCastAsync(
    type: Type[T], proxy: ObjectPrx | None, facet: str | None = None, context: dict[str, str] | None = None
) -> T | None:
    """
    Creates a new proxy from an existing proxy after confirming the target object's type via a remote invocation.

    Parameters
    ----------
    type : Type[T]
        The proxy target type.
    proxy : ObjectPrx | None
        The source proxy.
    facet : str | None, optional
        A facet name.
    context : dict[str, str] | None, optional
        The request context.

    Returns
    -------
    T | None
        A new proxy with the requested type and facet, or ``None`` if the source proxy is ``None`` or if the target
        object/facet does not support the requested type.
    """
    if proxy is None:
        return None
    if facet is not None:
        proxy = proxy.ice_facet(facet)
    b = await proxy.ice_isAAsync(type.ice_staticId(), context=context)
    return IcePy.ObjectPrx.newProxy(type, proxy) if b else None


class ObjectPrx(IcePy.ObjectPrx):
    """
    The base class for all Ice proxies.
    """

    @staticmethod
    def uncheckedCast(proxy: ObjectPrx | None, facet: str | None = None) -> ObjectPrx | None:
        """
        Creates a new proxy from an existing proxy.

        Parameters
        ----------
        proxy : ObjectPrx | None
            The source proxy.
        facet : str | None, optional
            A facet name.

        Returns
        -------
        ObjectPrx | None
            A new proxy with the requested facet, or ``None`` if the source proxy is ``None``.
        """
        return uncheckedCast(ObjectPrx, proxy, facet)

    @staticmethod
    def checkedCast(
        proxy: ObjectPrx | None, facet: str | None = None, context: dict[str, str] | None = None
    ) -> ObjectPrx | None:
        """
        Creates a new proxy from an existing proxy after confirming the target object's type via a remote invocation.

        Parameters
        ----------
        proxy : ObjectPrx | None
            The source proxy.
        facet : str | None, optional
            A facet name.
        context : dict[str, str] | None, optional
            The request context.

        Returns
        -------
        ObjectPrx | None
            A new proxy with the requested facet, or ``None`` if the source proxy is ``None`` or if the target
            object/facet does not support the requested type.
        """
        return checkedCast(ObjectPrx, proxy, facet, context)

    @staticmethod
    def checkedCastAsync(
        proxy: ObjectPrx, facet: str | None = None, context: dict[str, str] | None = None
    ) -> Awaitable[ObjectPrx | None]:
        """
        Creates a new proxy from an existing proxy after confirming the target object's type via a remote invocation.

        Parameters
        ----------
        proxy : ObjectPrx | None
            The source proxy.
        facet : str | None, optional
            A facet name.
        context : dict[str, str] | None, optional
            The request context.

        Returns
        -------
        ObjectPrx | None
            A new proxy with the requested facet, or ``None`` if the source proxy is ``None`` or if the target
            object/facet does not support the requested type.
        """
        return checkedCastAsync(ObjectPrx, proxy, facet, context)

    @staticmethod
    def ice_staticId() -> str:
        """
        Returns the Slice type ID associated with this type.

        Returns
        -------
        str
            The Slice type ID.
        """
        return "::Ice::Object"

    def ice_getCommunicator(self) -> Communicator:
        """
        Gets the communicator that created this proxy.

        Returns
        -------
        Communicator
            The communicator that created this proxy.
        """
        return super().ice_getCommunicator()

    def ice_isA(self, id: str, context: dict[str, str] | None = None) -> bool:
        """
        Tests whether this object supports a specific Slice interface.

        Parameters
        ----------
        id : str
            The type ID of the Slice interface to test against.
        context : dict[str, str] | None, optional
            The request context.

        Returns
        -------
        bool
            ``True`` if the target object implements the Slice interface specified by ``id``
            or implements a derived interface, ``False`` otherwise.
        """
        return Object._op_ice_isA.invoke(self, ((id,), context))

    def ice_isAAsync(self, id: str, context: dict[str, str] | None = None) -> Awaitable[bool]:
        """
        Tests whether this object supports a specific Slice interface.

        Parameters
        ----------
        id : str
            The type ID of the Slice interface to test against.
        context : dict[str, str] | None, optional
            The request context.

        Returns
        -------
        Awaitable[bool]
            An :class:`Awaitable` that completes when the invocation completes.
            It holds ``True`` if the target object implements the Slice interface specified by ``id``
            or implements a derived interface, ``False`` otherwise.
        """
        return Object._op_ice_isA.invokeAsync(self, ((id,), context))

    def ice_ping(self, context: dict[str, str] | None = None):
        """
        Tests whether the target object of this proxy can be reached.

        Parameters
        ----------
        context : dict[str, str] | None, optional
            The request context.
        """
        Object._op_ice_ping.invoke(self, ((), context))

    def ice_pingAsync(self, context: dict[str, str] | None = None) -> Awaitable[None]:
        """
        Tests whether the target object of this proxy can be reached.

        Parameters
        ----------
        context : dict[str, str] | None, optional
            The request context.

        Returns
        -------
        Awaitable[None]
            An :class:`Awaitable` that completes when the invocation completes.
        """
        return Object._op_ice_ping.invokeAsync(self, ((), context))

    def ice_ids(self, context: dict[str, str] | None = None) -> list[str]:
        """
        Returns the Slice interfaces supported by this object as a list of Slice type IDs.

        Parameters
        ----------
        context : dict[str, str] | None, optional
            The request context.

        Returns
        -------
        list[str]
            The Slice type IDs of the interfaces supported by this object, in alphabetical order.
        """
        return Object._op_ice_ids.invoke(self, ((), context))

    def ice_idsAsync(self, context: dict[str, str] | None = None) -> Awaitable[list[str]]:
        """
        Returns the Slice interfaces supported by this object as a list of Slice type IDs.

        Parameters
        ----------
        context : dict[str, str] | None, optional
            The request context.

        Returns
        -------
        Awaitable[list[str]]
            An :class:`Awaitable` that completes when the invocation completes.
            It holds the Slice type IDs of the interfaces supported by the target object, in alphabetical order.
        """
        return Object._op_ice_ids.invokeAsync(self, ((), context))

    def ice_id(self, context: dict[str, str] | None = None) -> str:
        """
        Returns the type ID of the most-derived Slice interface supported by this object.

        Parameters
        ----------
        context : dict[str, str] | None, optional
            The request context.

        Returns
        -------
        str
            The Slice type ID of the most-derived interface.
        """
        return Object._op_ice_id.invoke(self, ((), context))

    def ice_idAsync(self, context: dict[str, str] | None = None) -> Awaitable[str]:
        """
        Returns the type ID of the most-derived Slice interface supported by this object.

        Parameters
        ----------
        context : dict[str, str] | None, optional
            The request context.

        Returns
        -------
        Awaitable[str]
            An :class:`Awaitable` that completes when the invocation completes.
            It holds the Slice type ID of the most-derived interface.
        """
        return Object._op_ice_id.invokeAsync(self, ((), context))

    def ice_getIdentity(self) -> Identity:
        """
        Gets the identity embedded in this proxy.

        Returns
        -------
        Identity
            The identity of the target object.
        """
        return super().ice_getIdentity()

    def ice_identity(self, newIdentity: Identity) -> Self:
        """
        Creates a proxy that is identical to this proxy, except for the identity.

        Parameters
        ----------
        newIdentity : Identity
            The identity for the new proxy.

        Returns
        -------
        Self
            A proxy with the new identity.
        """
        return super().ice_identity(newIdentity)

    def ice_getContext(self) -> dict[str, str] | None:
        """
        Gets the per-proxy context for this proxy.

        Returns
        -------
        dict[str, str] | None
            The per-proxy context, or ``None`` if the proxy does not have a per-proxy context.
        """
        return super().ice_getContext()

    def ice_context(self, new_context: dict[str, str]) -> Self:
        """
        Creates a proxy that is identical to this proxy, except for the per-proxy context.

        Parameters
        ----------
        new_context : dict[str, str]
            The context for the new proxy.

        Returns
        -------
        Self
            A proxy with the new per-proxy context.
        """
        return super().ice_context(new_context)

    def ice_getFacet(self) -> str:
        """
        Gets the facet for this proxy.

        Returns
        -------
        str
            The facet for this proxy. If the proxy uses the default facet, the return value is the empty string.
        """
        return super().ice_getFacet()

    def ice_facet(self, new_facet: str) -> Self:
        """
        Creates a proxy that is identical to this proxy, except for the facet.

        Parameters
        ----------
        new_facet : str
            The facet for the new proxy.

        Returns
        -------
        Self
            A proxy with the new facet.
        """
        return super().ice_facet(new_facet)

    def ice_getAdapterId(self) -> str:
        """
        Gets the adapter ID for this proxy.

        Returns
        -------
        str
            The adapter ID. If the proxy does not have an adapter ID, the return value is the empty string.
        """
        return super().ice_getAdapterId()

    def ice_adapterId(self, newAdapterId: str) -> Self:
        """
        Creates a proxy that is identical to this proxy, except for the adapter ID.

        Parameters
        ----------
        newAdapterId : str
            The adapter ID for the new proxy.

        Returns
        -------
        Self
            A proxy with the new adapter ID.
        """
        return super().ice_adapterId(newAdapterId)

    def ice_getEndpoints(self) -> tuple[Endpoint, ...]:
        """
        Gets the endpoints used by this proxy.

        Returns
        -------
        tuple[Ice.Endpoint, ...]
            The endpoints used by this proxy.
        """
        return super().ice_getEndpoints()

    def ice_endpoints(self, newEndpoints: tuple[Endpoint, ...] | list[Endpoint]) -> Self:
        """
        Creates a proxy that is identical to this proxy, except for the endpoints.

        Parameters
        ----------
        newEndpoints : tuple[Ice.Endpoint, ...] | list[Ice.Endpoint]
            The endpoints for the new proxy.

        Returns
        -------
        Self
            A proxy with the new endpoints.
        """
        return super().ice_endpoints(newEndpoints)

    def ice_getLocatorCacheTimeout(self) -> int:
        """
        Gets the locator cache timeout of this proxy.

        Returns
        -------
        int
            The locator cache timeout value (in seconds).
        """
        return super().ice_getLocatorCacheTimeout()

    def ice_locatorCacheTimeout(self, timeout: int) -> Self:
        """
        Creates a proxy that is identical to this proxy, except for the locator cache timeout.

        Parameters
        ----------
        timeout : int
            The new locator cache timeout (in seconds).

        Returns
        -------
        Self
            A proxy with the new timeout.
        """
        return super().ice_locatorCacheTimeout(timeout)

    def ice_invocationTimeout(self, timeout: int) -> Self:
        """
        Creates a proxy that is identical to this proxy, except for the invocation timeout.

        Parameters
        ----------
        timeout : int
            The new invocation timeout (in milliseconds).

        Returns
        -------
        Self
            A proxy with the new timeout.
        """
        return super().ice_invocationTimeout(timeout)

    def ice_getInvocationTimeout(self) -> int:
        """
        Gets the invocation timeout of this proxy.

        Returns
        -------
        int
            The invocation timeout value (in milliseconds).
        """
        return super().ice_getInvocationTimeout()

    def ice_isConnectionCached(self) -> bool:
        """
        Determines whether this proxy caches connections.

        Returns
        -------
        bool
            ``True`` if this proxy caches connections, ``False`` otherwise.
        """
        return super().ice_isConnectionCached()

    def ice_connectionCached(self, newCache: bool) -> Self:
        """
        Creates a proxy that is identical to this proxy, except for connection caching.

        Parameters
        ----------
        newCache : bool
            ``True`` if the new proxy should cache connections, ``False`` otherwise.

        Returns
        -------
        Self
            A proxy with the specified caching policy.
        """
        return super().ice_connectionCached(newCache)

    def ice_getEndpointSelection(self) -> EndpointSelectionType:
        """
        Gets the endpoint selection policy for this proxy (randomly or ordered).

        Returns
        -------
        EndpointSelectionType
            The endpoint selection policy.
        """
        return super().ice_getEndpointSelection()

    def ice_endpointSelection(self, newType: EndpointSelectionType) -> Self:
        """
        Creates a proxy that is identical to this proxy, except for the endpoint selection policy.

        Parameters
        ----------
        newType : EndpointSelectionType
            The new endpoint selection policy.

        Returns
        -------
        Self
            A proxy with the specified endpoint selection policy.
        """
        return super().ice_endpointSelection(newType)

    def ice_encodingVersion(self, version: EncodingVersion) -> Self:
        """
        Creates a proxy that is identical to this proxy, except for the encoding used to marshal parameters.

        Parameters
        ----------
        version : EncodingVersion
            The encoding version to use to marshal request parameters.

        Returns
        -------
        Self
            A proxy with the specified encoding version.
        """
        return super().ice_encodingVersion(version)

    def ice_getEncodingVersion(self) -> EncodingVersion:
        """
        Gets the encoding version used to marshal request parameters.

        Returns
        -------
        EncodingVersion
            The encoding version.
        """
        return super().ice_getEncodingVersion()

    def ice_getRouter(self) -> RouterPrx | None:
        """
        Gets the router for this proxy.

        Returns
        -------
        RouterPrx | None
            The router for the proxy. If no router is configured for the proxy, the return value is ``None``.
        """
        return super().ice_getRouter()

    def ice_router(self, router: RouterPrx | None) -> Self:
        """
        Creates a proxy that is identical to this proxy, except for the router.

        Parameters
        ----------
        router : RouterPrx | None
            The router for the new proxy.

        Returns
        -------
        Self
            A proxy with the specified router.
        """
        return super().ice_router(router)

    def ice_getLocator(self) -> LocatorPrx | None:
        """
        Gets the locator for this proxy.

        Returns
        -------
        LocatorPrx | None
            The locator for this proxy. If no locator is configured, the return value is ``None``.
        """
        return super().ice_getLocator()

    def ice_locator(self, locator: LocatorPrx | None) -> Self:
        """
        Creates a proxy that is identical to this proxy, except for the locator.

        Parameters
        ----------
        locator : LocatorPrx | None
            The locator for the new proxy.

        Returns
        -------
        Self
            A proxy with the specified locator.
        """
        return super().ice_locator(locator)

    def ice_isCollocationOptimized(self) -> bool:
        """
        Determines whether this proxy uses collocation optimization.

        Returns
        -------
        bool
            ``True`` if the proxy uses collocation optimization, ``False`` otherwise.
        """
        return super().ice_isCollocationOptimized()

    def ice_collocationOptimized(self, collocated: bool) -> Self:
        """
        Creates a proxy that is identical to this proxy, except for collocation optimization.

        Parameters
        ----------
        collocated : bool
            ``True`` if the new proxy enables collocation optimization, ``False`` otherwise.

        Returns
        -------
        Self
            A proxy with the specified collocation optimization.
        """
        return super().ice_collocationOptimized(collocated)

    def ice_twoway(self) -> Self:
        """
        Creates a proxy that is identical to this proxy, but uses twoway invocations.

        Returns
        -------
        Self
            A proxy that uses twoway invocations.
        """
        return super().ice_twoway()

    def ice_isTwoway(self) -> bool:
        """
        Determines whether this proxy uses twoway invocations.

        Returns
        -------
        bool
            ``True`` if this proxy uses twoway invocations, ``False`` otherwise.
        """
        return super().ice_isTwoway()

    def ice_oneway(self) -> Self:
        """
        Creates a proxy that is identical to this proxy, but uses oneway invocations.

        Returns
        -------
        Self
            A proxy that uses oneway invocations.
        """
        return super().ice_oneway()

    def ice_isOneway(self) -> bool:
        """
        Determines whether this proxy uses oneway invocations.

        Returns
        -------
        bool
            ``True`` if this proxy uses oneway invocations, ``False`` otherwise.
        """
        return super().ice_isOneway()

    def ice_batchOneway(self) -> Self:
        """
        Creates a proxy that is identical to this proxy, but uses batch oneway invocations.

        Returns
        -------
        Self
            A proxy that uses batch oneway invocations.
        """
        return super().ice_batchOneway()

    def ice_isBatchOneway(self) -> bool:
        """
        Determines whether this proxy uses batch oneway invocations.

        Returns
        -------
        bool
            ``True`` if this proxy uses batch oneway invocations, ``False`` otherwise.
        """
        return super().ice_isBatchOneway()

    def ice_datagram(self) -> Self:
        """
        Creates a proxy that is identical to this proxy, but uses datagram invocations.

        Returns
        -------
        Self
            A proxy that uses datagram invocations.
        """
        return super().ice_datagram()

    def ice_isDatagram(self) -> bool:
        """
        Determines whether this proxy uses datagram invocations.

        Returns
        -------
        bool
            ``True`` if this proxy uses datagram invocations, ``False`` otherwise.
        """
        return super().ice_isDatagram()

    def ice_batchDatagram(self) -> Self:
        """
        Creates a proxy that is identical to this proxy, but uses batch datagram invocations.

        Returns
        -------
        Self
            A proxy that uses batch datagram invocations.
        """
        return super().ice_batchDatagram()

    def ice_isBatchDatagram(self) -> bool:
        """
        Determines whether this proxy uses batch datagram invocations.

        Returns
        -------
        bool
            ``True`` if this proxy uses batch datagram invocations, ``False`` otherwise.
        """
        return super().ice_isBatchDatagram()

    def ice_compress(self, compress: bool) -> Self:
        """
        Creates a proxy that is identical to this proxy, except for its compression setting which overrides the
        compression setting from the proxy endpoints.

        Parameters
        ----------
        compress : bool
            ``True`` enables compression for the new proxy, ``False`` disables compression.

        Returns
        -------
        Self
            A proxy with the specified compression override setting.
        """
        return super().ice_compress(compress)

    def ice_getCompress(self) -> bool | None:
        """
        Gets the compression override setting of this proxy.

        Returns
        -------
            The compression override setting. If ``None`` is returned, no override is set.
            Otherwise, ``True`` if compression is enabled, ``False`` otherwise.
        """
        return super().ice_getCompress()

    def ice_connectionId(self, connectionId: str) -> Self:
        """
        Creates a proxy that is identical to this proxy, except for its connection ID.

        Parameters
        ----------
        connectionId : str
            The connection ID for the new proxy. An empty string removes the connection ID.

        Returns
        -------
        Self
            A proxy with the specified connection ID.
        """
        return super().ice_connectionId(connectionId)

    def ice_getConnectionId(self) -> str:
        """
        Gets the connection ID of this proxy.

        Returns
        -------
        str
            The connection ID.
        """
        return super().ice_getConnectionId()

    def ice_fixed(self, connection: Connection) -> Self:
        """
        Creates a proxy that is identical to this proxy, except it's a fixed proxy bound to the given connection.

        Parameters
        ----------
        connection : Connection
            The fixed proxy connection.

        Returns
        -------
        Self
            A fixed proxy bound to the given connection.
        """
        return super().ice_fixed(connection)

    def ice_isFixed(self) -> bool:
        """
        Determines whether this proxy is a fixed proxy.

        Returns
        -------
        bool
            ``True`` if this proxy is a fixed proxy, ``False`` otherwise.
        """
        return super().ice_isFixed()

    def ice_getConnection(self) -> Connection | None:
        """
        Gets the connection for this proxy. If the proxy does not yet have an established connection,
        it first attempts to create a connection.

        Returns
        -------
        Connection | None
            The Connection for this proxy, or ``None`` when the target object is collocated.

        Notes
        -----
        You can call this function to establish a connection or associate the proxy with an existing
        connection and ignore the return value.
        """
        return super().ice_getConnection()

    def ice_getCachedConnection(self) -> Connection | None:
        """
        Gets the cached Connection for this proxy. If the proxy does not yet have an established connection,
        it does not attempt to create a connection.

        Returns
        -------
        Connection | None
            The cached connection for this proxy, or ``None`` if the proxy does not have an established connection.
        """
        return super().ice_getCachedConnection()

    def ice_flushBatchRequests(self) -> None:
        """
        Flushes any pending batched requests for this proxy. The call blocks until the flush is complete.
        """
        return super().ice_flushBatchRequests()

    def __repr__(self) -> str:
        return (
            f"{self.__class__.__module__}.{self.__class__.__qualname__}("
            f"communicator={self.ice_getCommunicator()!r}, "
            f"proxyString={self.ice_toString()!r})"
        )

    def __str__(self) -> str:
        return self.ice_toString()
