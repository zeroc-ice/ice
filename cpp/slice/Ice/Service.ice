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

/**
 *
 * The basic Ice module, containing all the Ice core functionality.
 *
 **/
module Ice
{
    
/**
 *
 * An application service. A Service is managed by a ServiceManager.
 * A Service's lifecycle has three stages. The ServiceManager will
 * invoke [init] on all services prior to calling [start], and will
 * invoke [stop] on all services when [shutdown] is called. The
 * order in which the services are invoked is not defined.
 *
 * <orderedlist>
 *
 * <listitem><para><important>init</important> - This is the
 * opportunity for the service to create a Communicator or Object
 * Adapter, register servants, etc.</para></listitem>
 *
 * <listitem><para><important>start</important> - Perform any
 * client-side activities which might result in an invocation
 * on a collocated service.</para></listitem>
 *
 * <listitem><para><important>start</important> - Destroy
 * Communicators, clean up resources, etc.</para></listitem>
 *
 * </orderedlist>
 *
 * <note><para>If the service requires an object adapter, it should
 * be created and activated in [init]. However, the service should
 * refrain from any client-side activities which might result in
 * an invocation on a collocated service, because the order of
 * service configuration is not defined and therefore the target
 * service may not be active yet. Client-side activities can be
 * safely performed in [start], as the ServiceManager guarantees
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
     * Initialize the Service. The <replaceable>communicator</replaceable>
     * argument is the Communicator instance created by the ServiceManager.
     * The service may use this instance, or may create its own as needed.
     * The advantage of using this Communicator instance is that
     * invocations between collocated services are optimized.
     *
     * <note><para>The ServiceManager owns this Communicator, and is
     * responsible for destroying it.</para></note>
     *
     * @param communicator The ServiceManager's Communicator instance.
     * @param args The service arguments.
     *
     * @see start
     *
     **/
    void init(Communicator communicator, StringSeq args);

    /**
     *
     * Start the Service.
     *
     **/
    void start();

    /**
     *
     * Stop the Service.
     *
     **/
    void stop();
};

/**
 *
 * An application server.
 *
 * @see Service
 *
 **/
interface ServiceManager
{
    /**
     *
     * Shutdown the server. This will cause [stop] to be invoked on
     * all configured services.
     *
     **/
    void shutdown();
};

};

#endif
