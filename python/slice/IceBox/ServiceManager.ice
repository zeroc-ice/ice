//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICEBOX_API"]]
[["cpp:doxygen:include:IceBox/IceBox.h"]]
[["cpp:header-ext:h"]]
[["cpp:include:IceBox/Config.h"]]

[["ice-prefix"]]

[["js:module:ice"]]

[["python:pkgdir:IceBox"]]

#include <Ice/BuiltinSequences.ice>

[["java:package:com.zeroc"]]

/**
 *
 * IceBox is an application server specifically for Ice
 * applications. IceBox can easily run and administer Ice services
 * that are dynamically loaded as a DLL, shared library, or Java
 * class.
 *
 **/
module IceBox
{

/**
 *
 * This exception is thrown if an attempt is made to start an
 * already-started service.
 *
 **/
exception AlreadyStartedException
{
}

/**
 *
 * This exception is thrown if an attempt is made to stop an
 * already-stopped service.
 *
 **/
exception AlreadyStoppedException
{
}

/**
 *
 * This exception is thrown if a service name does not refer
 * to an existing service.
 *
 **/
exception NoSuchServiceException
{
}

/**
 *
 * An Observer interface implemented by admin clients
 * interested in the status of services
 *
 * @see ServiceManager
 *
 **/
interface ServiceObserver
{
    void servicesStarted(Ice::StringSeq services);
    void servicesStopped(Ice::StringSeq services);
}

/**
 *
 * Administers a set of {@link Service} instances.
 *
 * @see Service
 *
 **/
interface ServiceManager
{
    /**
     *
     * Start an individual service.
     *
     * @param service The service name.
     *
     * @throws AlreadyStartedException If the service is already running.
     * @throws NoSuchServiceException If no service could be found with the given name.
     **/
    void startService(string service)
        throws AlreadyStartedException, NoSuchServiceException;

    /**
     *
     * Stop an individual service.
     *
     * @param service The service name.
     *
     * @throws AlreadyStoppedException If the service is already stopped.
     * @throws NoSuchServiceException If no service could be found with the given name.
     **/
    void stopService(string service)
        throws AlreadyStoppedException, NoSuchServiceException;

    /**
     *
     * Registers a new observer with the ServiceManager.
     *
     * @param observer The new observer
     *
     **/
    void addObserver(ServiceObserver* observer);

    /**
     *
     * Shut down all services. This causes stop to be invoked on all configured services.
     *
     **/
    void shutdown();
}

}
