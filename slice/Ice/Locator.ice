
// Copyright (c) ZeroC, Inc. All rights reserved.

#pragma once

[[cpp:dll-export(ICE_API)]]
[[cpp:doxygen:include(Ice/Ice.h)]]
[[cpp:header-ext(h)]]

[[suppress-warning(reserved-identifier)]]
[[js:module(ice)]]

[[python:pkgdir(Ice)]]

#include <Ice/BuiltinSequences.ice>
#include <Ice/Endpoint.ice>
#include <Ice/Identity.ice>
#include <Ice/Process.ice>

[cs:namespace(ZeroC)]
[java:package(com.zeroc)]
module Ice
{
    /// This exception is thrown when a server tries to register endpoints for an object adapter that is already active.
    exception AdapterAlreadyActiveException
    {
    }

    /// This exception is thrown when an object adapter was not found.
    exception AdapterNotFoundException
    {
    }

    /// This exception is thrown when the provided replica group is invalid.
    exception InvalidReplicaGroupIdException
    {
    }

    /// This exception is thrown when an Ice object was not found.
    exception ObjectNotFoundException
    {
    }

    /// This exception is thrown when a server was not found.
    exception ServerNotFoundException
    {
    }

    interface LocatorRegistry;

    /// Client applications use Locator to resolve locations and well-known proxies. The Locator object also allows
    /// server applications to retrieve a proxy to the LocatorRegistry object.
    interface Locator
    {
#ifdef __SLICE2CS__
        /// Finds an object by identity and facet and returns a proxy that provides a location or endpoint(s) that can
        /// be used to reach the object using the ice1 protocol.
        /// @param id The identity.
        /// @param facet The facet. A null value is equivalent to the empty string.
        /// @return An ice1 proxy that provides a location or endpoint(s), or null if an object with the requested
        /// identity and facet was not found.
        /// @throws ObjectNotFoundException Thrown if an object with the requested identity and facet was not found. The
        /// caller should treat this exception like a null return value.
        idempotent Object? findObjectById(Identity id, tag(1) string? facet);

        /// Finds an object adapter by id and returns a proxy that provides the object adapter's endpoint(s). This
        /// operation is for object adapters using the ice1 protocol.
        /// @param id The adapter ID.
        /// @return An ice1 proxy with the adapter's endpoint(s), or null if an object adapter with adapter ID `id' was
        /// not found.
        /// @throws AdapterNotFoundException Thrown if an object adapter with this adapter ID was not found. The caller
        /// should treat this exception like a null return value.
        idempotent Object? findAdapterById(string id);

        /// Gets the locator registry.
        /// @return The locator registry, or null if this locator has no registry.
        idempotent LocatorRegistry? getRegistry();

        /// Resolves the location of a proxy that uses the ice2 protocol.
        /// @param location The location to resolve.
        /// @return A sequence of one or more endpoints when the location can be resolved, and an empty sequence of
        /// endpoints when the location cannot be resolved.
        idempotent EndpointDataSeq resolveLocation(StringSeq location);

        /// Resolves the well-known object with the given identity and facet. This object must be reachable using the
        /// ice2 protocol.
        /// @param identity The identity of the well-known Ice object.
        /// @param facet The facet of the well-known Ice object.
        /// @return If the locator can locate the well-known object, either a sequence of one or more endpoints or
        /// a non-empty location. Otherwise, an empty sequence of endpoints and an empty location.
        idempotent (EndpointDataSeq endpoints, StringSeq location) resolveWellKnownProxy(
            Identity identity,
            string facet);

#else
        [amd] [nonmutating] [cpp:const] idempotent Object? findObjectById(Identity id)
            throws ObjectNotFoundException;

        [amd] [nonmutating] [cpp:const] idempotent Object? findAdapterById(string id)
            throws AdapterNotFoundException;

        [cpp:const] idempotent LocatorRegistry? getRegistry();
#endif
    }

    /// A server application registers the endpoints of its indirect object adapters with the LocatorRegistry object.
    interface LocatorRegistry
    {
#ifdef __SLICE2CS__
        /// Registers the endpoints of an object adapter that uses the ice2 protocol.
        /// @param adapterId The adapter ID.
        /// @param replicaGroupId The replica group ID. It is set to the empty string when the object adapter does not
        /// belong to a replica group.
        /// @param endpoints A sequence of one or more endpoints. The locator considers an object adapter to be active
        /// after it has registered its endpoints.
        /// @throws AdapterNotFoundException Thrown if the locator only allows registered object adapters to register
        /// their active endpoints and no object adapter with this adapter ID and replica group ID (if applicable) was
        /// registered with the locator.
        /// @throws InvalidArgumentException Thrown if any of the provided arguments is invalid, such as an empty
        /// adapter ID, empty endpoint sequence or adapter ID and replica group ID are inconsistent.
        void registerAdapterEndpoints(string adapterId, string replicaGroupId, EndpointDataSeq endpoints);

        /// Registers or unregisters the endpoints of an object adapter that uses the ice1 protocol.
        /// @param id The adapter ID.
        /// @param proxy A dummy direct proxy created by the object adapter that provides the object adapter's
        /// endpoints. The locator considers an object adapter to be active after it has registered its endpoints. When
        /// proxy is null, the endpoints are unregistered and the locator considers the object adapter inactive.
        /// @throws AdapterNotFoundException Thrown if the locator only allows registered object adapters to register
        /// their active endpoints and no object adapter with this adapter ID was registered with the locator.
        /// @throws AdapterAlreadyActiveException Thrown if an object adapter with the same adapter ID has already
        /// registered its endpoints.
        // Note: idempotent is not quite correct, and kept only for backwards compatibility with old implementations.
        idempotent void setAdapterDirectProxy(string id, Object? proxy);

        /// Registers or unregisters the endpoints of an object adapter that uses the ice1 protocol. This object adapter
        /// is member of a replica group.
        /// @param adapterId The adapter ID.
        /// @param replicaGroupId The replica group ID.
        /// @param proxy A dummy direct proxy created by the object adapter that provides the object adapter's
        /// endpoints. The locator considers an object adapter to be active after it has registered its endpoints. When
        /// proxy is null, the endpoints are unregistered and the locator considers the object adapter inactive.
        /// @throws AdapterNotFoundException Thrown if the locator only allows registered object adapters to register
        /// their active endpoints and no object adapter with this adapter ID was registered with the locator.
        /// @throws AdapterAlreadyActiveException Thrown if an object adapter with the same adapter ID has already
        /// @throws InvalidReplicaGroupIdException Thrown if the given replica group does not match the replica group
        /// associated with the adapter ID in the locator's database.
        // Note: idempotent is not quite correct, and kept only for backwards compatibility with old implementations.
        idempotent void setReplicatedAdapterDirectProxy(string adapterId, string replicaGroupId, Object? proxy);

        /// Registers a proxy for a server's Process object.
        /// @param serverId The server ID.
        /// @param proxy A proxy for the server's Process object.
        /// @throws ServerNotFoundException Thrown if the locator does not know a server with this server ID.
        idempotent void setServerProcessProxy(string serverId, Process proxy);

        /// Unregisters the endpoints of an object adapter that uses the ice2 protocol, or does nothing if this object
        /// adapter is not active.
        /// @param adapterId The adapter ID.
        /// @param replicaGroupId The replica group ID. It is set to the empty string when the object adapter does not
        /// belong to a replica group.
        /// @throws AdapterNotFoundException Thrown if the locator only allows registered object adapters to register
        /// their active endpoints and no object adapter with this adapter ID and replica group ID (if applicable) was
        /// registered with the locator.
        /// @throws InvalidArgumentException Thrown if any of the provided arguments is invalid.
        idempotent void unregisterAdapterEndpoints(string adapterId, string replicaGroupId);

#else
        [amd] idempotent void setAdapterDirectProxy(string id, Object? proxy)
            throws AdapterNotFoundException, AdapterAlreadyActiveException;

        [amd] idempotent void setReplicatedAdapterDirectProxy(string adapterId, string replicaGroupId, Object? proxy)
            throws AdapterNotFoundException, AdapterAlreadyActiveException, InvalidReplicaGroupIdException;

        [amd] idempotent void setServerProcessProxy(string id, Process proxy)
            throws ServerNotFoundException;
#endif
    }

    /// This interface is implemented by services that implement the Ice::Locator interface, and is advertised as an Ice
    /// object with the identity `Ice/LocatorFinder'. This allows clients to retrieve the locator proxy with just the
    /// endpoint information of the service.
    interface LocatorFinder
    {
        /// Gets the locator proxy implemented by the service hosting this finder object. The proxy might point to
        /// several replicas.
        /// @return The locator proxy.
        Locator getLocator();
    }
}
