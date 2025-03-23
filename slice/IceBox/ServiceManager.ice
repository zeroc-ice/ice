// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICEBOX_API"]]
[["cpp:doxygen:include:IceBox/IceBox.h"]]
[["cpp:header-ext:h"]]

[["cpp:include:IceBox/Config.h"]]

[["java:package:com.zeroc"]]
[["js:module:@zeroc/ice"]]
[["python:pkgdir:IceBox"]]

#include "Ice/BuiltinSequences.ice"

/// Host multiple independent services in the same Ice server.
module IceBox
{
    /// The exception that is thrown whenan attempt is made to start an already-started service.
    exception AlreadyStartedException
    {
    }

    /// The exception that is thrown whenan attempt is made to stop an already-stopped service.
    exception AlreadyStoppedException
    {
    }

    /// The exception that is thrown whena service name does not refer to an existing service.
    exception NoSuchServiceException
    {
    }

    /// An Observer interface implemented by admin clients interested in the status of services.
    /// @see ServiceManager
    interface ServiceObserver
    {
        /// Receives the names of the services that were started.
        /// @param services The names of the services.
        void servicesStarted(Ice::StringSeq services);

        /// Receives the names of the services that were stopped.
        /// @param services The names of the services.
        void servicesStopped(Ice::StringSeq services);
    }

    /// Administers a set of IceBox Service instances.
    /// @see Service
    interface ServiceManager
    {
        /// Start an individual service.
        /// @param service The service name.
        /// @throws AlreadyStartedException If the service is already running.
        /// @throws NoSuchServiceException If no service could be found with the given name.
        void startService(string service)
            throws AlreadyStartedException, NoSuchServiceException;

        /// Stop an individual service.
        /// @param service The service name.
        /// @throws AlreadyStoppedException If the service is already stopped.
        /// @throws NoSuchServiceException If no service could be found with the given name.
        void stopService(string service)
            throws AlreadyStoppedException, NoSuchServiceException;

        /// Registers a new observer with the ServiceManager.
        /// @param observer The new observer.
        void addObserver(ServiceObserver* observer);

        /// Shut down all services. This causes stop to be invoked on all configured services.
        void shutdown();
    }
}
