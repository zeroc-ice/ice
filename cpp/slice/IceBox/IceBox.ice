// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef ICE_BOX_ICE_BOX_ICE
#define ICE_BOX_ICE_BOX_ICE

#include <Ice/BuiltinSequences.ice>
#include <Ice/CommunicatorF.ice>
#include <Ice/PropertiesF.ice>

/**
 *
 * &IceBox; is an application server specifically for &Ice;
 * applications. &IceBox; can easily run and administer Ice services
 * that are dynamically loaded as a DLL, shared library, or Java
 * class.
 *
 **/
module IceBox
{
    
/**
 *
 * Indicates a failure occurred. For example, if a service encounters
 * an error during initialization, or if the service manager is unable
 * to load a service executable.
 *
 **/
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
 * Base interface for an application service managed by a [ServiceManager].
 *
 * @see ServiceManager
 * @see Service
 * @see FreezeService
 *
 **/
local interface ServiceBase
{
    /**
     *
     * Stop the service.
     *
     **/
    void stop();
};

/**
 *
 * A standard application service managed by a [ServiceManager].
 *
 * @see ServiceBase
 *
 **/
local interface Service extends ServiceBase
{
    /**
     *
     * Start the service. The given communicator is created by the
     * [ServiceManager] for use by the service. This communicator may
     * also be used by other services, depending on the service
     * configuration.
     *
     * <note><para>The [ServiceManager] owns this communicator, and is
     * responsible for destroying it.</para></note>
     *
     * @param name The service's name, as determined by the
     * configuration.
     *
     * @param communicator A communicator for use by the service.
     *
     * @param args The service arguments that were not converted into
     * properties.
     *
     * @throws FailureException Raised if [start] failed.
     *
     **/
    void start(string name, Ice::Communicator communicator, Ice::StringSeq args);
};

/**
 * 
 * A Freeze application service managed by a [ServiceManager].
 *
 * @see ServiceBase
 * 
 */
local interface FreezeService extends ServiceBase
{
    /**
     *
     * Start the service. The given communiator is created by the
     * [ServiceManager] for use by the service. This communicator may
     * also be used by other services, depending on the service
     * configuration. The database environment is created by the
     * [ServiceManager] for the exclusive use of the service.
     *
     * <note><para>The [ServiceManager] owns the communiator and the
     * database environment, and is responsible for destroying
     * them.</para></note>
     *
     * @param name The service's name, as determined by the
     * configuration.
     *
     * @param communicator A communiator for use by the service.
     *
     * @param args The service arguments that were not converted into
     * properties.
     *
     * @param envName The name of the Freeze database environment.
     *
     * @throws FailureException Raised if [start] failed.
     *
     **/
    void start(string name, Ice::Communicator communicator, Ice::StringSeq args, string envName);
};

/**
 *
 * Administers a set of [Service] instances.
 *
 * @see Service
 *
 **/
interface ServiceManager
{
    /**
     *
     * Shutdown all services. This will cause [Service::stop] to be
     * invoked on all configured services.
     *
     **/
    void shutdown();
};

};

#endif
