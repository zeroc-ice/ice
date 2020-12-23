//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[cpp:dll-export(ICEBOX_API)]]
[[cpp:doxygen:include(IceBox/IceBox.h)]]
[[cpp:header-ext(h)]]
[[cpp:include(IceBox/Config.h)]]

[[suppress-warning(reserved-identifier)]]
[[js:module(ice)]]

[[python:pkgdir(IceBox)]]

#include <Ice/BuiltinSequences.ice>

[[java:package(com.zeroc)]]
[cs:namespace(ZeroC)]
/// IceBox is an application server for Ice applications. IceBox can load IceBox services packaged as DLLs, .NET
/// assemblies, Java classes and similar.
module IceBox
{
    /// This exception is thrown if an attempt is made to start an already-started service.
    exception AlreadyStartedException
    {
    }

    /// This exception is thrown if an attempt is made to stop an already-stopped service.
    exception AlreadyStoppedException
    {
    }

    /// This exception is thrown if a service name does not refer to an existing service.
    exception NoSuchServiceException
    {
    }

    /// This exception is thrown when a service fails to start or stop.
    exception ServiceException
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

    /// Administers a set of {@link Service} instances.
    /// @see Service
    interface ServiceManager
    {
        /// Starts an individual service.
        /// @param service The service name.
        /// @throws AlreadyStartedException If the service is already running.
        /// @throws NoSuchServiceException If no service could be found with the given name.
        void startService(string service)
            throws AlreadyStartedException, NoSuchServiceException;

        /// Stops an individual service.
        /// @param service The service name.
        /// @throws AlreadyStoppedException If the service is already stopped.
        /// @throws NoSuchServiceException If no service could be found with the given name.
        void stopService(string service)
            throws AlreadyStoppedException, NoSuchServiceException;

        /// Registers a new observer with the ServiceManager.
        /// @param observer The new observer.
        void addObserver(ServiceObserver observer);

        /// Shuts down all services. This causes stop to be invoked on all configured services.
        void shutdown();
    }
}
