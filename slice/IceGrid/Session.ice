//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[cpp:dll-export(ICEGRID_API)]]
[[cpp:doxygen:include(IceGrid/IceGrid.h)]]
[[cpp:header-ext(h)]]
[[cpp:include(IceGrid/Config.h)]]

[[suppress-warning(reserved-identifier)]]
[[js:module(ice)]]

[[python:pkgdir(IceGrid)]]

#include <Glacier2/Session.ice>
#include <IceGrid/Exception.ice>

[[java:package(com.zeroc)]]
[cs:namespace(ZeroC)]
module IceGrid
{
    /// A session object is used by IceGrid clients to allocate and
    /// release objects. Client sessions are created either via the
    /// {@link Registry} object or via the registry client <code>SessionManager</code>
    /// object.
    ///
    /// @see Registry
    interface Session : Glacier2::Session
    {
        /// Keep the session alive. Clients should call this operation
        /// regularly to prevent the server from reaping the session.
        ///
        /// @see Registry#getSessionTimeout
        idempotent void keepAlive();

        /// Allocate an object. Depending on the allocation timeout, this
        /// operation might hang until the object is available or until the
        /// timeout is reached.
        ///
        /// @param id The identity of the object to allocate.
        ///
        /// @return The proxy of the allocated object.
        ///
        /// @throws ObjectNotRegisteredException Raised if the object with
        /// the given identity is not registered with the registry.
        ///
        /// @throws AllocationException Raised if the object can't be
        /// allocated.
        ///
        /// @see #setAllocationTimeout
        /// @see #releaseObject
        [amd] Object* allocateObjectById(Ice::Identity id)
            throws ObjectNotRegisteredException, AllocationException;

        /// Allocate an object with the given type. Depending on the
        /// allocation timeout, this operation can block until an object
        /// becomes available or until the timeout is reached.
        ///
        /// @param type The type of the object.
        ///
        /// @return The proxy of the allocated object.
        ///
        /// @throws AllocationException Raised if the object could not be allocated.
        ///
        /// @see #setAllocationTimeout
        /// @see #releaseObject
        [amd] Object* allocateObjectByType(string type)
            throws AllocationException;

        /// Release an object that was allocated using <code>allocateObjectById</code> or
        /// <code>allocateObjectByType</code>.
        ///
        /// @param id The identity of the object to release.
        ///
        /// @throws ObjectNotRegisteredException Raised if the object with
        /// the given identity is not registered with the registry.
        ///
        /// @throws AllocationException Raised if the given object can't be
        /// released. This might happen if the object isn't allocatable or
        /// isn't allocated by the session.
        void releaseObject(Ice::Identity id)
            throws ObjectNotRegisteredException, AllocationException;

        /// Set the allocation timeout. If no objects are available for an
        /// allocation request, a call to <code>allocateObjectById</code> or
        /// <code>allocateObjectByType</code> will block for the duration of this
        /// timeout.
        ///
        /// @param timeout The timeout in milliseconds.
        idempotent void setAllocationTimeout(int timeout);
    }
}
