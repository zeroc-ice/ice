// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICEBOX_API"]]
[["cpp:doxygen:include:IceBox/IceBox.h"]]
[["cpp:header-ext:h"]]

[["cpp:include:IceBox/Config.h"]]

[["js:module:@zeroc/ice"]]
[["python:pkgdir:IceBox"]]

#include "Ice/BuiltinSequences.ice"

/// Host multiple independent services in the same Ice server.
["java:identifier:com.zeroc.IceBox"]
module IceBox
{
    /// The exception that is thrown when attempting to start a service that is already running.
    exception AlreadyStartedException
    {
    }

    /// The exception that is thrown when attempting to stop a service that is already stopped.
    exception AlreadyStoppedException
    {
    }

    /// The exception that is thrown when a service name does not refer to a known service.
    exception NoSuchServiceException
    {
    }

    /// Observes the status of services in an IceBox server.
    /// @remark This interface is implemented by admin tools that monitor the IceBox server.
    /// @see ServiceManager#addObserver
    interface ServiceObserver
    {
        /// Receives the names of the services that were started.
        /// @param services The names of the services that were started.
        void servicesStarted(Ice::StringSeq services);

        /// Receives the names of the services that were stopped.
        /// @param services The names of the services that were stopped.
        void servicesStopped(Ice::StringSeq services);
    }

    /// Administers the services of an IceBox server.
    interface ServiceManager
    {
        /// Starts a service.
        /// @param service The service name.
        /// @throws AlreadyStartedException Thrown when the service is already running.
        /// @throws NoSuchServiceException Thrown when IceBox does not know a service named @p service.
        void startService(string service)
            throws AlreadyStartedException, NoSuchServiceException;

        /// Stops a service.
        /// @param service The service name.
        /// @throws AlreadyStoppedException Thrown when the service is already stopped.
        /// @throws NoSuchServiceException Thrown when IceBox does not know a service named @p service.
        void stopService(string service)
            throws AlreadyStoppedException, NoSuchServiceException;

        /// Registers a new observer with this service manager.
        /// @param observer The new observer.
        void addObserver(ServiceObserver* observer);

        /// Shuts down all services.
        void shutdown();
    }
}
