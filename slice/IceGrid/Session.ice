// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICEGRID_API"]]
[["cpp:doxygen:include:IceGrid/IceGrid.h"]]
[["cpp:header-ext:h"]]

[["cpp:include:IceGrid/Config.h"]]

[["js:module:@zeroc/ice"]]
[["python:pkgdir:IceGrid"]]

#include "Exception.ice"
#include "Glacier2/Session.ice"

["java:identifier:com.zeroc.IceGrid"]
module IceGrid
{
    interface Registry; // For doc-comments.

    /// Represents a session object used by IceGrid clients to allocate and release objects. Client sessions are created
    /// either via the {@link Registry} object or via the registry client `SessionManager` object.
    interface Session extends Glacier2::Session
    {
        /// Keeps the session alive.
        ["deprecated:As of Ice 3.8, there is no need to call this operation, and its implementation does nothing."]
        idempotent void keepAlive();

        /// Allocates an object.
        /// @param id The identity of the object to allocate.
        /// @return A proxy to the allocated object. This proxy is never null.
        /// @throws ObjectNotRegisteredException Thrown when an object with the given identity is not registered with
        /// the registry.
        /// @throws AllocationException Thrown when the allocation fails.
        /// @see #setAllocationTimeout
        /// @see #releaseObject
        ["amd"] Object* allocateObjectById(Ice::Identity id)
            throws ObjectNotRegisteredException, AllocationException;

        /// Allocates an object with the given type.
        /// @param type The type of the object.
        /// @return A proxy to the allocated object. This proxy is never null.
        /// @throws AllocationException Thrown when the allocation fails.
        /// @see #setAllocationTimeout
        /// @see #releaseObject
        ["amd"] Object* allocateObjectByType(string type)
            throws AllocationException;

        /// Releases an object that was allocated using {@link allocateObjectById} or {@link allocateObjectByType}.
        /// @param id The identity of the object to release.
        /// @throws ObjectNotRegisteredException Thrown when an object with the given identity is not registered with
        /// the registry.
        /// @throws AllocationException Thrown when the object can't be released. This can happen when the object is not
        /// allocatable or is not allocated by this session.
        void releaseObject(Ice::Identity id)
            throws ObjectNotRegisteredException, AllocationException;

        /// Sets the allocation timeout. When no object is immediately available for an allocation request, the
        /// implementation of {@link allocateObjectById} and {@link allocateObjectByType} waits for the duration of
        /// this timeout.
        /// @param timeout The timeout in milliseconds.
        idempotent void setAllocationTimeout(int timeout);
    }
}
