// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_BOX_ICE_BOX_ICE
#define ICE_BOX_ICE_BOX_ICE

#include <Ice/BuiltinSequences.ice>
#include <Ice/CommunicatorF.ice>
#include <Ice/PropertiesF.ice>
#include <Freeze/DB.ice>

module IceBox
{
    
/**
 *
 * Used to indicate failure. For example, if a service encounters
 * an error during initialization, or if the service manager is
 * unable to load a service executable.
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
 * The * [ServiceManager] will invoke [init] on all services prior to
 * calling [start], and will invoke [stop] on all services when
 * [ServiceManager::shutdown] is called. The order in which the
 * services are invoked is not defined. The service lifecycle
 * operations are described below:
 *
 * <itemizedlist>
 *
 * <listitem><para>[init] - This is the opportunity for the service to
 * create a Communicator or object adapter, register servants,
 * etc.</para></listitem>
 *
 * <listitem><para>[start] - Perform any client-side activities which
 * might result in an invocation on a collocated service.
 * </para></listitem>
 *
 * <listitem><para>[stop] - Destroy Communicators, deactivate Object
 * Adapters, clean up resources, etc. The [ServiceManager] guarantees
 * that [stop] will be invoked on all services whose [init] has been
 * invoked.</para></listitem>
 *
 * </itemizedlist>
 *
 * <note><para>If the service requires an object adapter, it should be
 * created and activated in [init]. However, the service should
 * refrain from any client-side activities which might result in an
 * invocation on a collocated service, because the order of service
 * configuration is not defined and therefore the target service may
 * not be active yet. Client-side activities can be safely performed
 * in [start], as the [ServiceManager] guarantees that all services
 * will be configured before [start] is invoked.  </para></note>
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

#if 0
    //
    // Potential future operations
    //
    Ice::StringSeq getParameters() throws ServiceNotExist; // Need better return type
    void setParameter(string param, string value) throws FailureException; // Use a different except?
#endif
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
     * Initialize the service. The given Communicator is created by
     * the [ServiceManager]. It may be shared by other services
     * depending on the service configuration.
     *
     * <note><para>The [ServiceManager] owns this Communicator, and is
     * responsible for destroying it.</para></note>
     *
     * @param name The service's name, as determined by the
     * configuration.
     *
     * @param communicator The [ServiceManager]'s Communicator
     * instance.
     *
     * @param args The service arguments which were not converted into
     * properties.
     *
     * @throws FailureException Raised if [init] failed.
     *
     * @see start
     *
     **/
    void start(string name, Ice::Communicator communicator, Ice::StringSeq args)
        throws FailureException;

};

/**
 * 
 * A Freeze application service managed by the [ServiceManager]. It takes
 * care of the the initialization and shutdown of the Freeze database for
 * the application.
 *
 * @see ServiceBase
 * 
 */
local interface FreezeService extends ServiceBase
{
    /**
     *
     * Initialize the service. The given Communicator is created by
     * the [ServiceManager]. It may be shared by other services
     * depending on the service configuration.
     *
     * <note><para>The [ServiceManager] owns this Communicator, and is
     * responsible for destroying it.</para></note>
     *
     * @param name The service's name, as determined by the
     * configuration.
     *
     * @param communicator The [ServiceManager]'s Communicator
     * instance.
     *
     * @param args The service arguments which were not converted into
     * properties.
     *
     * @param dbEnv The Freeze database environment.
     *
     * @throws FailureException Raised if [init] failed.
     *
     * @see ServiceBase
     *
     **/
    void start(string name, Ice::Communicator communicator, Ice::StringSeq args, Freeze::DBEnvironment dbEnv)
        throws FailureException;
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
    // ML: I don't like the name shutdown. Shouldn't this be stopAll(), as below?
    void shutdown();

#if 0
    //
    // Potential future operations
    //
    Ice::StringSeq getServices();
    void start(string service, string exec, Ice::StringSeq args) throws FailureException;
    void stop(string service) throws ServiceNotExist; // Allow service to override this?
    void stopAll();
    Ice::StringSeq getParameters(string service) throws ServiceNotExist; // Need better return type
    void setParameter(string service, string param, string value) throws ServiceNotExist, FailureException; // Use a different except?
#endif
};

};

#endif
