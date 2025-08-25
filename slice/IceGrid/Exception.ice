// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICEGRID_API"]]
[["cpp:doxygen:include:IceGrid/IceGrid.h"]]
[["cpp:header-ext:h"]]

[["cpp:include:IceGrid/Config.h"]]

[["js:module:@zeroc/ice"]]

#include "Ice/BuiltinSequences.ice"
#include "Ice/Identity.ice"

["java:identifier:com.zeroc.IceGrid"]
module IceGrid
{
    /// The exception that is thrown when IceGrid does not know an application with the provided name.
    exception ApplicationNotExistException
    {
        /// The name of the application.
        string name;
    }

    /// The exception that is thrown when IceGrid does not know a server with the provided server ID.
    exception ServerNotExistException
    {
        /// The server ID.
        string id;
    }

    /// The exception that is thrown when a server failed to start.
    exception ServerStartException
    {
        /// The server ID.
        string id;

        /// The reason for the failure.
        string reason;
    }

    /// The exception that is thrown when a server failed to stop.
    exception ServerStopException
    {
        /// The server ID.
        string id;

        /// The reason for the failure.
        string reason;
    }

    /// The exception that is thrown when IceGrid does not know an object adapter with the provided adapter ID.
    exception AdapterNotExistException
    {
        /// The adapter ID.
        string id;
    }

    /// The exception that is thrown when a well-known object is already registered.
    exception ObjectExistsException
    {
        /// The identity of the object.
        Ice::Identity id;
    }

    /// The exception that is thrown when a well-known object is not registered.
    exception ObjectNotRegisteredException
    {
        /// The identity of the object.
        Ice::Identity id;
    }

    /// The exception that is thrown when IceGrid does not know a node with the provided name.
    exception NodeNotExistException
    {
        /// The node name.
        string name;
    }

    /// The exception that is thrown when IceGrid does not know a registry with the provided name.
    exception RegistryNotExistException
    {
        /// The registry name.
        string name;
    }

    /// The exception that is thrown when IceGrid cannot deploy a server.
    exception DeploymentException
    {
        /// The reason for the failure.
        string reason;
    }

    /// The exception that is thrown when IceGrid cannot reach a node.
    exception NodeUnreachableException
    {
        /// The name of the node that is not reachable.
        string name;

        /// The reason why the node couldn't be reached.
        string reason;
    }

    /// The exception that is thrown when IceGrid cannot reach a server.
    exception ServerUnreachableException
    {
        /// The id of the server that is not reachable.
        string name;

        /// The reason why the server couldn't be reached.
        string reason;
    }

    /// The exception that is thrown when IceGrid cannot reach a registry.
    exception RegistryUnreachableException
    {
        /// The name of the registry that is not reachable.
        string name;

        /// The reason why the registry couldn't be reached.
        string reason;
    }

    /// The exception that is thrown when an unknown signal is sent to a server.
    exception BadSignalException
    {
        /// The details of the unknown signal.
        string reason;
    }

    /// The exception that is thrown when the registry update lock cannot be acquired.
    exception AccessDeniedException
    {
        /// The id of the user holding the lock (if any).
        string lockUserId;
    }

    /// The exception that is thrown when the allocation of an object failed.
    exception AllocationException
    {
        /// The reason why the object couldn't be allocated.
        string reason;
    }

    /// The exception that is thrown when the request to allocate an object times out.
    exception AllocationTimeoutException extends AllocationException
    {
    }

    /// The exception that is thrown when a client is not allowed to create a session.
    exception PermissionDeniedException
    {
        /// The reason why permission was denied.
        string reason;
    }

    /// The exception that is thrown when an observer is already registered with the registry.
    /// @see AdminSession#setObservers
    /// @see AdminSession#setObserversByIdentity
    exception ObserverAlreadyRegisteredException
    {
        /// The identity of the observer.
        Ice::Identity id;
    }

    /// The exception that is thrown when a log file is not available.
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
