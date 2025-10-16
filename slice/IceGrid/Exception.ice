// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICEGRID_API"]]
[["cpp:doxygen:include:IceGrid/IceGrid.h"]]
[["cpp:header-ext:h"]]

[["cpp:include:IceGrid/Config.h"]]

[["js:module:@zeroc/ice"]]

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
}
