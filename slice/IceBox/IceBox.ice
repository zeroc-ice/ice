// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc"]]
[["cpp:include:IceBox/Config.h"]]

#include <Ice/BuiltinSequences.ice>
#include <Ice/CommunicatorF.ice>
#include <Ice/PropertiesF.ice>
#include <Ice/SliceChecksumDict.ice>

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
 * This exception is a general failure notification. It is thrown
 * for errors such as a service encountering an error during
 * initialization, or the service manager being unable
 * to load a service executable.
 *
 **/
["cpp:ice_print"]
local exception FailureException
{
    /**
     *
     * The reason for the failure.
     *
     **/
    string reason;
};


/**
 *
 * This exception is thrown if an attempt is made to start an
 * already-started service.
 *
 **/
exception AlreadyStartedException
{
};

/**
 *
 * This exception is thrown if an attempt is made to stop an
 * already-stopped service.
 *
 **/
exception AlreadyStoppedException
{
};

/**
 *
 * This exception is thrown if a service name does not refer
 * to an existing service.
 *
 **/
exception NoSuchServiceException
{
};

/**
 *
 * An application service managed by a {@link ServiceManager}.
 *
 **/
local interface Service
{
    /**
     *
     * Start the service. The given communicator is created by the
     * {@link ServiceManager} for use by the service. This communicator may
     * also be used by other services, depending on the service
     * configuration.
     *
     * <p class="Note">The {@link ServiceManager} owns this communicator, and is
     * responsible for destroying it.
     *
     * @param name The service's name, as determined by the
     * configuration.
     *
     * @param communicator A communicator for use by the service.
     *
     * @param args The service arguments that were not converted into
     * properties.
     *
     * @throws FailureException Raised if {@link #start} failed.
     *
     **/
    void start(string name, Ice::Communicator communicator, Ice::StringSeq args);

    /**
     *
     * Stop the service.
     *
     **/
    void stop();
};


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
};


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
     * Returns the checksums for the IceBox Slice definitions.
     *
     * @return A dictionary mapping Slice type ids to their checksums.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent Ice::SliceChecksumDict getSliceChecksums();

    /**
     *
     * Start an individual service.
     *
     * @param service The service name.
     *
     **/
    void startService(string service)
        throws AlreadyStartedException, NoSuchServiceException;

    /**
     *
     * Stop an individual service.
     *
     * @param service The service name.
     *
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
     * Shut down all services. This causes {@link Service#stop} to be
     * invoked on all configured services.
     *
     **/
    void shutdown();
};

};

