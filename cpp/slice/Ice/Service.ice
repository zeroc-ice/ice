// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SERVICE_ICE
#define ICE_SERVICE_ICE

#include <Ice/BuiltinSequences.ice>
#include <Ice/CommunicatorF.ice>
#include <Ice/PropertiesF.ice>

module Ice
{
    
/**
 *
 * Used by a service to indicate failure.
 *
 **/
local exception ServiceFailureException
{
};

/**
 *
 * An application service managed by a [ServiceManager].
 * The [ServiceManager] will invoke [init] on all services prior
 * to calling [start], and will invoke [stop] on all services when
 * [ServiceManager::shutdown] is called. The order in which the
 * services are invoked is not defined. The service lifecycle
 * operations are described below:
 *
 * <itemizedlist>
 *
 * <listitem><para>[init] - This is the opportunity for the service
 * to create a Communicator or Object Adapter, register servants,
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
 * <note><para>If the service requires an object adapter, it should
 * be created and activated in [init]. However, the service should
 * refrain from any client-side activities which might result in
 * an invocation on a collocated service, because the order of
 * service configuration is not defined and therefore the target
 * service may not be active yet. Client-side activities can be
 * safely performed in [start], as the [ServiceManager] guarantees
 * that all services will be configured before [start] is invoked.
 * </para></note>
 *
 * @see ServiceManager
 *
 **/
local interface Service
{
    /**
     *
     * Initialize the service. The given Communicator is created by the
     * [ServiceManager]. The service may use this instance, or may
     * create its own as needed. The advantage of using this Communicator
     * instance is that invocations between collocated services are optimized.
     *
     * <note><para>The [ServiceManager] owns this Communicator, and is
     * responsible for destroying it.</para></note>
     *
     * @param name The service's name, as determined by the configuration.
     *
     * @param communicator The [ServiceManager]'s Communicator instance.
     *
     * @param properties The property set representing the service's
     * command-line arguments of the form
     * [--<replaceable>name</replaceable>.key=value].
     *
     * @param args The service arguments which were not converted into
     * properties.
     *
     * @throws ServiceFailureException Raised if [init] failed.
     *
     * @see start
     *
     **/
    void init(string name, Communicator communicator, Properties properties, StringSeq args)
        throws ServiceFailureException;

    /**
     *
     * Start the service.
     *
     * @throws ServiceFailureException Raised if [start] failed.
     *
     **/
    void start()
        throws ServiceFailureException;

    /**
     *
     * Stop the service.
     *
     **/
    void stop();
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
     * Shutdown the server. This will cause [Service::stop] to be invoked on
     * all configured services.
     *
     **/
    void shutdown();
};

};

#endif
