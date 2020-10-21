//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[cpp:doxygen:include(IceGrid/IceGrid.h)]]
[[cpp:header-ext(h)]]
[[cpp:include(IceGrid/Config.h)]]

[[suppress-warning(reserved-identifier)]]
[[js:module(ice)]]

[[python:pkgdir(IceGrid)]]

#ifndef ICE_BUILDING_ICEGRIDDB
[[cpp:dll-export(ICEGRID_API)]]
#endif

#include <Ice/Identity.ice>
#include <Ice/BuiltinSequences.ice>

[[java:package(com.zeroc)]]
[cs:namespace(ZeroC)]
module IceGrid
{
    /// This exception is raised if an application does not exist.
    exception ApplicationNotExistException
    {
        /// The name of the application.
        string name;
    }

    /// This exception is raised if a server does not exist.
    exception ServerNotExistException
    {
        /// The identifier of the server.
        string id;
    }

    /// This exception is raised if a server failed to start.
    exception ServerStartException
    {
        /// The identifier of the server.
        string id;

        /// The reason for the failure.
        string reason;
    }

    /// This exception is raised if a server failed to stop.
    exception ServerStopException
    {
        /// The identifier of the server.
        string id;

        /// The reason for the failure.
        string reason;
    }

    /// This exception is raised if an adapter does not exist.
    exception AdapterNotExistException
    {
        /// The id of the object adapter.
        string id;
    }

    /// This exception is raised if an object already exists.
    exception ObjectExistsException
    {
        /// The identity of the object.
        Ice::Identity id;
    }

    /// This exception is raised if an object is not registered.
    exception ObjectNotRegisteredException
    {
        /// The identity of the object.
        Ice::Identity id;
    }

    /// This exception is raised if a node does not exist.
    exception NodeNotExistException
    {
        /// The node name.
        string name;
    }

    /// This exception is raised if a registry does not exist.
    exception RegistryNotExistException
    {
        /// The registry name.
        string name;
    }

    /// An exception for deployment errors.
    exception DeploymentException
    {
        /// The reason for the failure.
        string reason;
    }

    /// This exception is raised if a node could not be reached.
    exception NodeUnreachableException
    {
        /// The name of the node that is not reachable.
        string name;

        /// The reason why the node couldn't be reached.
        string reason;
    }

    /// This exception is raised if a server could not be reached.
    exception ServerUnreachableException
    {
        /// The id of the server that is not reachable.
        string name;

        /// The reason why the server couldn't be reached.
        string reason;
    }

    /// This exception is raised if a registry could not be reached.
    exception RegistryUnreachableException
    {
        /// The name of the registry that is not reachable.
        string name;

        /// The reason why the registry couldn't be reached.
        string reason;
    }

    /// This exception is raised if an unknown signal was sent to
    /// to a server.
    exception BadSignalException
    {
        /// The details of the unknown signal.
        string reason;
    }

    /// This exception is raised if a registry lock wasn't
    /// acquired or is already held by a session.
    exception AccessDeniedException
    {
        /// The id of the user holding the lock (if any).
        string lockUserId;
    }

    /// This exception is raised if the allocation of an object failed.
    exception AllocationException
    {
        /// The reason why the object couldn't be allocated.
        string reason;
    }

    /// This exception is raised if the request to allocate an object times
    /// out.
    exception AllocationTimeoutException : AllocationException
    {
    }

    /// This exception is raised if a client is denied the ability to create
    /// a session with IceGrid.
    exception PermissionDeniedException
    {
        /// The reason why permission was denied.
        string reason;
    }

    /// This exception is raised if an observer is already registered with
    /// the registry.
    ///
    /// @see AdminSession#setObservers
    /// @see AdminSession#setObserversByIdentity
    exception ObserverAlreadyRegisteredException
    {
        /// The identity of the observer.
        Ice::Identity id;
    }

    /// This exception is raised if a file is not available.
    ///
    /// @see AdminSession#openServerStdOut
    /// @see AdminSession#openServerStdErr
    /// @see AdminSession#openNodeStdOut
    /// @see AdminSession#openNodeStdErr
    /// @see AdminSession#openRegistryStdOut
    /// @see AdminSession#openRegistryStdErr
    exception FileNotAvailableException
    {
        /// The reason for the failure.
        string reason;
    }
}
