// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]
[["cpp:source-include:Ice/Process.h"]]

[["java:package:com.zeroc"]]
[["js:module:@zeroc/ice"]]
[["python:pkgdir:Ice"]]

#include "Locator.ice"

module Ice
{
    interface Process;

    /// This exception is thrown when a server application tries to register endpoints for an object adapter that is
    /// already active.
    exception AdapterAlreadyActiveException
    {
    }

    /// This exception is thrown when the provided replica group is invalid.
    exception InvalidReplicaGroupIdException
    {
    }

    /// This exception is thrown when a server was not found.
    exception ServerNotFoundException
    {
    }

    /// A server application registers the endpoints of its indirect object adapters with the LocatorRegistry object.
    interface LocatorRegistry
    {
        /// Registers or unregisters the endpoints of an object adapter.
        /// @param id The adapter ID.
        /// @param proxy A dummy proxy created by the object adapter. @p proxy carries the object adapter's endpoints.
        /// The locator considers an object adapter to be active after it has registered its endpoints.
        /// When @p proxy is null, the endpoints are unregistered and the locator considers the object adapter inactive.
        /// @throws AdapterNotFoundException Thrown if the locator only allows registered object adapters to register
        /// their endpoints and no object adapter with this adapter ID was registered with the locator.
        /// @throws AdapterAlreadyActiveException Thrown if an object adapter with the same adapter ID has already
        /// registered its endpoints.
        // Note: idempotent is not correct but kept for backwards compatibility with old implementations.
        ["amd"] idempotent void setAdapterDirectProxy(string id, Object* proxy)
            throws AdapterNotFoundException, AdapterAlreadyActiveException;

        /// Registers or unregisters the endpoints of an object adapter. This object adapter is a member of a replica
        /// group.
        /// @param adapterId The adapter ID.
        /// @param replicaGroupId The replica group ID.
        /// @param proxy A dummy proxy created by the object adapter. @p proxy carries the object adapter's endpoints.
        /// The locator considers an object adapter to be active after it has registered its endpoints. When @p proxy is
        /// null, the endpoints are unregistered and the locator considers the object adapter inactive.
        /// @throws AdapterNotFoundException Thrown if the locator only allows registered object adapters to register
        /// their endpoints and no object adapter with this adapter ID was registered with the locator.
        /// @throws AdapterAlreadyActiveException Thrown if an object adapter with the same adapter ID has already
        /// registered its endpoints.
        /// @throws InvalidReplicaGroupIdException Thrown if the given replica group does not match the replica group
        /// associated with the adapter ID in the locator's database.
        // Note: idempotent is not correct but kept for backwards compatibility with old implementations.
        ["amd"] idempotent void setReplicatedAdapterDirectProxy(string adapterId, string replicaGroupId, Object* proxy)
            throws AdapterNotFoundException, AdapterAlreadyActiveException, InvalidReplicaGroupIdException;

        /// Registers a proxy to the {@link Process} object of a server application.
        /// @param id The server ID.
        /// @param proxy A proxy to the Process object of the server. This proxy is never null.
        /// @throws ServerNotFoundException Thrown if the locator does not know a server application with this server
        /// ID.
        ["amd"] idempotent void setServerProcessProxy(string id, Process* proxy)
            throws ServerNotFoundException;
    }
}
