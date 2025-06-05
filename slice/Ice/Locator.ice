// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["cpp:source-include:Ice/LocatorRegistry.h"]]

[["js:module:@zeroc/ice"]]
[["python:pkgdir:Ice"]]

#include "Identity.ice"

["java:identifier:com.zeroc.Ice"]
module Ice
{
    /// The exception that is thrown by a {@link Locator} implementation when it cannot find an object adapter with the
    /// provided adapter ID.
    exception AdapterNotFoundException
    {
    }

    /// The exception that is thrown by a {@link Locator} implementation when it cannot find an object with the provided
    /// identity.
    exception ObjectNotFoundException
    {
    }

    interface LocatorRegistry;

    /// Client applications use the {@link Locator} object to resolve Ice indirect proxies. This object also allows
    /// server applications to retrieve a proxy to the associated {@link LocatorRegistry} object where they can register
    /// their object adapters.
    interface Locator
    {
        /// Finds an object by identity and returns a dummy proxy with the endpoint(s) that can be used to reach this
        /// object. This dummy proxy may be an indirect proxy that requires further resolution using
        /// {@link findAdapterById}.
        /// @param id The identity.
        /// @return A dummy proxy, or null if an object with the requested identity was not found.
        /// @throws ObjectNotFoundException Thrown when an object with the requested identity was not found. The caller
        /// should treat this exception like a null return value.
        ["amd"] ["cpp:const"]
        idempotent Object* findObjectById(Identity id)
            throws ObjectNotFoundException;

        /// Finds an object adapter by adapter ID and returns a dummy proxy with the object adapter's endpoint(s).
        /// @param id The adapter ID.
        /// @return A dummy proxy with the adapter's endpoints, or null if an object adapter with @p id was not found.
        /// @throws AdapterNotFoundException Thrown when an object adapter with this adapter ID was not found. The
        /// caller should treat this exception like a null return value.
        ["amd"] ["cpp:const"]
        idempotent Object* findAdapterById(string id)
            throws AdapterNotFoundException;

        /// Gets a proxy to the locator registry.
        /// @return A proxy to the locator registry, or null if this locator has no associated registry.
        ["cpp:const"]
        idempotent LocatorRegistry* getRegistry();
    }

    /// Provides access to a {@link Locator} object via a fixed identity.
    /// A LocatorFinder is always registered with identity `Ice/LocatorFinder`. This allows clients to obtain the
    /// associated Locator proxy with just the endpoint information of the object. For example, you can use the
    /// LocatorFinder proxy `Ice/LocatorFinder:tcp -h somehost -p 4061` to get the Locator proxy
    /// `MyIceGrid/Locator:tcp -h somehost -p 4061`.
    interface LocatorFinder
    {
        /// Gets a proxy to the associated {@link Locator}. The proxy might point to several replicas.
        /// @return The locator proxy. This proxy is never null.
        Locator* getLocator();
    }
}
