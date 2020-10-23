
// Copyright (c) ZeroC, Inc. All rights reserved.

#pragma once

[[cpp:dll-export(ICE_API)]]
[[cpp:doxygen:include(Ice/Ice.h)]]
[[cpp:header-ext(h)]]

[[suppress-warning(reserved-identifier)]]
[[js:module(ice)]]

[[python:pkgdir(Ice)]]

#include <Ice/BuiltinSequences.ice>
#include <Ice/Identity.ice>
#include <Ice/Process.ice>
#include <Ice/Protocol.ice>

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

    /// This exception is thrown when a server was notfound.
    exception ServerNotFoundException
    {
    }

    interface LocatorRegistry;

    /// Client applications use Locator to resolve locations and well-known proxies. The Locator object also allows
    /// server applications to retrieve a proxy to the LocatorRegistry object.
    interface Locator
    {
#ifdef __SLICE2CS__
        /// Finds an object by identity and returns a proxy that provides a location or endpoint(s) that can be used
        /// to reach the object. Calling this operation is equivalent to calling
        /// resolveWellKnownProxy(id, Protocol::Ice1).
        /// @param id The identity.
        /// @return A proxy that provides a location or endpoint(s) or null if an object with identity `id' was
        /// not found.
        /// @throws ObjectNotFoundException Thrown if an object with identity `id' was not found. The caller should
        /// treat this exception like a null return value.
        [deprecate(use resolveWellKnownProxy)]
        idempotent Object? findObjectById(Identity id);

        /// Finds an object adapter by id and returns a proxy that provides the object adapter's endpoint(s). Calling
        /// this operation is equivalent to calling resolveLocation([id], Protocol::Ice1).
        /// @param id The adapter ID.
        /// @return A proxy with the adapter's endpoint(s) or null if an object adapter with adapter ID `id' was
        /// not found.
        /// @throws AdapterNotFoundException Thrown if an object adapter with this adapter ID was not found. The caller
        /// should treat this exception like a null return value.
        [deprecate(use resolveLocation)]
        idempotent Object? findAdapterById(string id);

        /// Gets the locator registry.
        /// @return The locator registry, or null if this locator has no registry.
        idempotent LocatorRegistry? getRegistry();

        /// Resolves the location of a proxy.
        /// @param location The location to resolve.
        /// @param protocol The protocol of the proxy being resolved.
        /// @return A direct proxy that provides the endpoint(s) plus optionally a revised location for the supplied
        /// location (ice2 only), or null when the location cannot be resolved. When not null, the proxy has the
        /// specified protocol.
        idempotent Object? resolveLocation(StringSeq location, Protocol protocol);

        /// Locates the well-known object with the given identity.
        /// @param identity The identity of the well-known Ice object.
        /// @param protocol The protocol of the proxy being resolved.
        /// @return A proxy that provides the location or endpoint(s) of the well-known object, or null if an object
        /// with identity `identity' was not found.
        idempotent Object? resolveWellKnownProxy(Identity identity, Protocol protocol);

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
        /// Registers the endpoints of an object adapter. Since Ice 4.0.
        /// @param adapterId The adapter ID.
        /// @param replicaGroupId The replica groupd ID. It is set to the empty string when the object adapter does not
        /// belong to a replica group.
        /// @param proxy A dummy direct proxy created by the object adapter that provides the object adapter's
        /// endpoints. The locator considers an object adapter to be active after it has registered its endpoints.
        /// @throws AdapterAlreadyActiveException Thrown if an object adapter with the same adapter ID has already
        /// registered its endpoints.
        /// @throws AdapterNotFoundException Thrown if the locator only allows registered object adapters to register
        /// their active endpoints and no object adapter with this adapter ID and replica group ID (if applicable) was
        /// registered with the locator.
        /// @throws InvalidReplicaGroupIdException Thrown if the given replica group does not match the replica group
        /// associated with the adapter ID in the locator's database.
        void registerAdapterEndpoints(string adapterId, string replicaGroupId, Object proxy);

        /// Registers or unregisters the endpoints of an object adapter. When proxy is not null, calling this operation
        /// is equivalent to calling registerAdapterEndpoints(id, "", proxy). When proxy is null, calling this operation
        /// is equivalent to calling unregisterAdapterEndpoints(id, "", Protocol::Ice1).
        /// @param id The adapter ID.
        /// @param proxy A dummy direct proxy created by the object adapter that provides the object adapter's
        /// endpoints. The locator considers an object adapter to be active after it has registered its endpoints. When
        /// proxy is null, the endpoints are unregistered and the locator considers the object adapter inactive.
        /// @throws AdapterNotFoundException Thrown if the locator only allows registered object adapters to register
        /// their active endpoints and no object adapter with this adapter ID was registered with the locator.
        /// @throws AdapterAlreadyActiveException Thrown if an object adapter with the same adapter ID has already
        /// registered its endpoints.
        // Note: idempotent is not quite correct, and kept only for backwards compatibility with old implementations.
        [deprecate(use registerAdapterEndpoints or unregisterAdapterEndpoints)]
        idempotent void setAdapterDirectProxy(string id, Object? proxy);

        /// Registers or unregisters the endpoints of an object adapter that is a member of a replica group.  When proxy
        /// is not null, calling this operation is equivalent to calling
        /// registerReplicagedAdapterEndpoints(adapterId, replicaGroupId, proxy). When proxy is null, calling this
        /// operation is equivalent to calling unregisterAdapterEndpoints(adapterId, replicaGroupId, Protocol::Ice1).
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
        [deprecate(use registerAdapterEndpoints or unregisterAdapterEndpoints)]
        // Note: idempotent is not quite correct, and kept only for backwards compatibility with old implementations.
        idempotent void setReplicatedAdapterDirectProxy(string adapterId, string replicaGroupId, Object? proxy);

        /// Registers a proxy for a server's Process object.
        /// @param serverId The server ID.
        /// @param proxy A proxy for the server's Process object.
        /// @throws ServerNotFoundException Thrown if the locator does not know a server with this server ID.
        idempotent void setServerProcessProxy(string serverId, Process proxy);

        /// Unregisters the endpoints of an object adapter, if this object adapter has registered its endpoints. Since
        /// Ice 4.0.
        /// @param adapterId The adapter ID.
        /// @param replicaGroupId The replica groupd ID. It is set to the empty string when the object adapter does not
        /// belong to a replica group.
        /// @param protocol The protocol of object adapter being unregistered.
        /// @throws AdapterNotFoundException Thrown if the locator only allows registered object adapters to register
        /// their active endpoints and no object adapter with this adapter ID and replica group ID (if applicable) was
        /// registered with the locator.
        /// @throws InvalidReplicaGroupIdException Thrown if the given replica group does not match the replica group
        /// associated with the adapter ID in the locator's database.
        idempotent void unregisterAdapterEndpoints(string adapterId, string replicaGroupId, Protocol protocol);

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
