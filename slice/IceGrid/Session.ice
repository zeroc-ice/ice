// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICEGRID_API"]]
[["cpp:doxygen:include:IceGrid/IceGrid.h"]]
[["cpp:header-ext:h"]]
[["cpp:include:IceGrid/Config.h"]]

[["js:module:@zeroc/ice"]]

[["python:pkgdir:IceGrid"]]

#include "Glacier2/Session.ice"
#include "Exception.ice"

[["java:package:com.zeroc"]]

module IceGrid
{
    interface Registry; // So that doc-comments can link to `IceGrid::Registry`.

    /// A session object is used by IceGrid clients to allocate and release objects. Client sessions are created either
    /// via the {@link Registry} object or via the registry client <code>SessionManager</code> object.
    /// @see Registry
    interface Session extends Glacier2::Session
    {
        /// Keep the session alive.
        /// As of Ice 3.8, there is no need to call this operation, and its implementation does nothing.
        ["deprecated"]
        idempotent void keepAlive();

        /// Allocate an object. Depending on the allocation timeout, this operation might hang until the object is
        /// available or until the timeout is reached.
        /// @param id The identity of the object to allocate.
        /// @return The proxy of the allocated object. The returned proxy is never null.
        /// @throws ObjectNotRegisteredException Raised if the object with the given identity is not registered with
        /// the registry.
        /// @throws AllocationException Raised if the object can't be allocated.
        /// @see #setAllocationTimeout
        /// @see #releaseObject
        ["amd"] Object* allocateObjectById(Ice::Identity id)
            throws ObjectNotRegisteredException, AllocationException;

        /// Allocate an object with the given type. Depending on the allocation timeout, this operation can block until
        /// an object becomes available or until the timeout is reached.
        /// @param type The type of the object.
        /// @return The proxy of the allocated object. The returned proxy is never null.
        /// @throws AllocationException Raised if the object could not be allocated.
        /// @see #setAllocationTimeout
        /// @see #releaseObject
        ["amd"] Object* allocateObjectByType(string type)
            throws AllocationException;

        /// Release an object that was allocated using <code>allocateObjectById</code> or
        /// <code>allocateObjectByType</code>.
        /// @param id The identity of the object to release.
        /// @throws ObjectNotRegisteredException Raised if the object with the given identity is not registered with
        /// the registry.
        /// @throws AllocationException Raised if the given object can't be released. This might happen if the object
        /// isn't allocatable or isn't allocated by the session.
        void releaseObject(Ice::Identity id)
            throws ObjectNotRegisteredException, AllocationException;

        /// Set the allocation timeout. If no objects are available for an allocation request, a call to
        /// <code>allocateObjectById</code> or <code>allocateObjectByType</code> will block for the duration of this
        /// timeout.
        /// @param timeout The timeout in milliseconds.
        idempotent void setAllocationTimeout(int timeout);
    }
}
