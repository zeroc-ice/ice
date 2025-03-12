// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["cpp:source-include:Ice/Process.h"]]

[["js:module:@zeroc/ice"]]

[["python:pkgdir:Ice"]]

#include "Identity.ice"

[["java:package:com.zeroc"]]

module Ice
{
    interface Process;

    /// This exception is raised if an adapter cannot be found.
    exception AdapterNotFoundException
    {
    }

    /// This exception is raised if the replica group provided by the server is invalid.
    exception InvalidReplicaGroupIdException
    {
    }

    /// This exception is raised if a server tries to set endpoints for an adapter that is already active.
    exception AdapterAlreadyActiveException
    {
    }

    /// This exception is raised if an object cannot be found.
    exception ObjectNotFoundException
    {
    }

    /// This exception is raised if a server cannot be found.
    exception ServerNotFoundException
    {
    }

    interface LocatorRegistry;

    /// The Ice locator interface. This interface is used by clients to lookup adapters and objects. It is also used by
    /// servers to get the locator registry proxy. Note: The {@link Locator} interface is intended to be used
    /// by Ice internals and by locator implementations. Regular user code should not attempt to use any functionality
    /// of this interface directly.
    interface Locator
    {
        /// Find an object by identity and return a proxy that contains the adapter ID or endpoints which can be used to
        /// access the object.
        /// @param id The identity.
        /// @return The proxy, or null if the object is not active.
        /// @throws ObjectNotFoundException Raised if the object cannot be found.
        ["amd"] ["cpp:const"] idempotent Object* findObjectById(Identity id)
            throws ObjectNotFoundException;

        /// Find an adapter by id and return a proxy that contains its endpoints.
        /// @param id The adapter id.
        /// @return The adapter proxy, or null if the adapter is not active.
        /// @throws AdapterNotFoundException Raised if the adapter cannot be found.
        ["amd"] ["cpp:const"] idempotent Object* findAdapterById(string id)
            throws AdapterNotFoundException;

        /// Get the locator registry.
        /// @return The locator registry.
        ["cpp:const"] idempotent LocatorRegistry* getRegistry();
    }

    /// The Ice locator registry interface. This interface is used by servers to register adapter endpoints with the
    /// locator. Note: The {@link LocatorRegistry} interface is intended to be used by Ice internals and by
    /// locator implementations. Regular user code should not attempt to use any functionality of this interface
    /// directly.
    interface LocatorRegistry
    {
        /// Set the adapter endpoints with the locator registry.
        /// @param id The adapter id.
        /// @param proxy The adapter proxy (a dummy direct proxy created by the adapter). The direct proxy contains the
        /// adapter endpoints. The proxy can be null, typically during adapter deactivation.
        /// @throws AdapterNotFoundException Raised if the adapter cannot be found, or if the locator only allows
        /// registered adapters to set their active proxy and the adapter is not registered with the locator.
        /// @throws AdapterAlreadyActiveException Raised if an adapter with the same id is already active.
        ["amd"] idempotent void setAdapterDirectProxy(string id, Object* proxy)
            throws AdapterNotFoundException, AdapterAlreadyActiveException;

        /// Set the adapter endpoints with the locator registry.
        /// @param adapterId The adapter id.
        /// @param replicaGroupId The replica group id.
        /// @param proxy The adapter proxy (a dummy direct proxy created by the adapter). The direct proxy contains the
        /// adapter endpoints. The proxy can be null, typically during adapter deactivation.
        /// @throws AdapterNotFoundException Raised if the adapter cannot be found, or if the locator only allows
        /// registered adapters to set their active proxy and the adapter is not registered with the locator.
        /// @throws AdapterAlreadyActiveException Raised if an adapter with the same id is already active.
        /// @throws InvalidReplicaGroupIdException Raised if the given replica group doesn't match the one registered
        /// with the locator registry for this object adapter.
        ["amd"] idempotent void setReplicatedAdapterDirectProxy(string adapterId, string replicaGroupId, Object* proxy)
            throws AdapterNotFoundException, AdapterAlreadyActiveException, InvalidReplicaGroupIdException;

        /// Set the process proxy for a server.
        /// @param id The server id.
        /// @param proxy The process proxy. The proxy is never null.
        /// @throws ServerNotFoundException Raised if the server cannot be found.
        ["amd"] idempotent void setServerProcessProxy(string id, Process* proxy)
            throws ServerNotFoundException;
    }

    /// This interface should be implemented by services implementing the <code>Ice::Locator interface</code>. It should
    /// be advertised through an Ice object with the identity <code>'Ice/LocatorFinder'</code>. This allows clients to
    /// retrieve the locator proxy with just the endpoint information of the service.
    interface LocatorFinder
    {
        /// Get the locator proxy implemented by the process hosting this finder object. The proxy might point to
        /// several replicas. This proxy is never null.
        /// @return The locator proxy.
        Locator* getLocator();
    }
}
