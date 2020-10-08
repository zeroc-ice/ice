
// Copyright (c) ZeroC, Inc. All rights reserved.

#pragma once

[[cpp:dll-export:ICE_API]]
[[cpp:doxygen:include:Ice/Ice.h]]
[[cpp:header-ext:h]]

[[suppress-warning:reserved-identifier]]
[[js:module:ice]]

[[python:pkgdir:Ice]]

#include <Ice/Identity.ice>
#include <Ice/Process.ice>

[[java:package:com.zeroc]]
[cs:namespace:ZeroC]
module Ice
{
    /// This exception is thrown when an object adapter was not found.
    exception AdapterNotFoundException
    {
    }

    /// This exception is thrown when the provided proxy is invalid.
    exception InvalidProxyException
    {
    }

    /// This exception is thrown when the provided replica group is invalid.
    exception InvalidReplicaGroupIdException
    {
    }

    /// This exception is thrown when a server tries to register endpoints for an object adapter that is already active.
    exception AdapterAlreadyActiveException
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

    /// The Ice locator interface. This interface is used by clients to lookup adapters and objects. It is also used by
    /// servers to get the locator registry proxy.
    interface Locator
    {
        /// Finds an object by identity and returns a proxy that provides a location or endpoint(s) that can be used
        /// to reach the object.
        /// @param id The identity.
        /// @return A proxy that provides a location or endpoint(s) or null if an object with identity <c>id</c> was
        /// not found.
        /// @throws ObjectNotFoundException Thrown if an object with identity <c>id</c> was not found. This exception
        /// is equivalent to a null return value.
        [amd] [nonmutating] [cpp:const] idempotent Object? findObjectById(Identity id)
            throws ObjectNotFoundException;

        /// Finds an object adapter by id and returns a proxy that provides the object adapter's endpoint(s).
        /// @param id The adapter ID.
        /// @return A proxy with the adapter's endpoint(s) or null if an object adapter with adapter ID <c>id</c> was
        /// not found.
        /// @throws AdapterNotFoundException Thrown if an object adapter with this adapter ID was not found. This
        /// exception is equivalent to a null return value.
        [amd] [nonmutating] [cpp:const] idempotent Object? findAdapterById(string id)
            throws AdapterNotFoundException;

        /// Gets the locator registry.
        /// @return The locator registry, or null if this locator has no registry.
        [nonmutating] [cpp:const] idempotent LocatorRegistry? getRegistry();
    }

    /// The Ice locator registry interface. This interface is used by a server to register the endpoints of its object
    /// adapters with the locator.
    interface LocatorRegistry
    {
        /// Registers or unregisters the endpoints of an object adapter.
        /// @param id The adapter ID.
        /// @param proxy A dummy direct proxy created by the object adapter that provides the object adapter's
        /// endpoints. The locator considers an object adapter to be active after it has registered its endpoints. When
        /// proxy is null, the endpoints are unregistered and the locator considers the object adapter inactive.
        /// @throws AdapterNotFoundException Thrown if the locator only allows registered object adapters to register
        /// their active endpoints and no object adapter with this adapter ID was registered with the locator.
        /// @throws AdapterAlreadyActiveException Thrown if an object adapter with the same adapter ID has already
        /// registered its endpoints.
        [amd] idempotent void setAdapterDirectProxy(string id, Object? proxy)
            throws AdapterNotFoundException, AdapterAlreadyActiveException;

        /// Registers or unregisters the endpoints of an object adapter that is a member of a replica group.
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
        [amd] idempotent void setReplicatedAdapterDirectProxy(string adapterId, string replicaGroupId, Object? proxy)
            throws AdapterNotFoundException, AdapterAlreadyActiveException, InvalidReplicaGroupIdException;

        /// Registers a proxy for a server's Process object.
        /// @param id The server ID.
        /// @param proxy A proxy for the server's Process object.
        /// @throws ServerNotFoundException Thrown if the locator does not know a server with this server ID.
        [amd] idempotent void setServerProcessProxy(string id, Process proxy)
            throws ServerNotFoundException;
    }

    /// This interface is implemented by services that implement the Ice::Locator interface, and is advertised as an
    /// Ice object with the identity `Ice/LocatorFinder'. This allows clients to retrieve the locator proxy with just
    /// the endpoint information of the service.
    interface LocatorFinder
    {
        /// Gets the locator proxy implemented by the service hosting this finder object. The proxy might point to
        /// several replicas.
        /// @return The locator proxy.
        Locator getLocator();
    }
}
