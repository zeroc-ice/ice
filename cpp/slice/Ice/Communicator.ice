// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef ICE_COMMUNICATOR_ICE
#define ICE_COMMUNICATOR_ICE

#include <Ice/LoggerF.ice>
#include <Ice/StatsF.ice>
#include <Ice/ObjectAdapterF.ice>
#include <Ice/PropertiesF.ice>
#include <Ice/ObjectFactoryF.ice>
#include <Ice/RouterF.ice>
#include <Ice/LocatorF.ice>
#include <Ice/PluginF.ice>

/**
 *
 * The &Ice; core library. Among many other features, the &Ice; core
 * library manages all the communication tasks using an efficient
 * protocol (including protocol compression and support for both TCP
 * and UDP), provides a thread pool for multi-threaded servers, and
 * additional functionality that supports high scalability.
 *
 **/
module Ice
{
    
/**
 *
 * The central object in &Ice;. One or more communicators can be
 * instantiated for an &Ice; application. Communicator instantiation
 * is language specific, and not specified in Slice code.
 *
 * @see Logger
 * @see Stats
 * @see ObjectAdapter
 * @see Properties
 * @see ObjectFactory
 *
 **/
local interface Communicator
{
    /**
     *
     * Destroy the communicator. This operation calls [shutdown]
     * implicitly.  Calling [destroy] cleans up memory, and shuts down
     * this communicator's client functionality. Subsequent calls to
     * [destroy] are ignored.
     *
     * @see shutdown
     *
     **/
    void destroy();

    /**
     *
     * Shuts down this communicator's server functionality, including
     * the deactivation of all object adapters. Subsequent calls to
     * [shutdown] are ignored.
     *
     * <note><para> After [shutdown] returns, no new requests are
     * processed. However, requests that have been started before
     * [shutdown] was called might still be active. You can use
     * [waitForShutdown] to wait for the completion of all
     * requests. </para></note>
     *
     * @see destroy
     * @see waitForShutdown
     * @see ObjectAdapter::deactivate
     *
     **/
    void shutdown();

    /**
     *
     * Wait until this communicator's server functionality has shut
     * down completely. Calling [shutdown] initiates shutdown, and
     * [waitForShutdown] only returns when all outstanding requests
     * have completed. A typical use of this operation is to call it
     * from the main thread, which then waits until some other thread
     * calls [shutdown]. After shutdown is complete, the main thread
     * returns and can do some cleanup work before it finally calls
     * [destroy] to also shut down the client functionality, and then
     * exits the application.
     *
     * @see shutdown
     * @see destroy
     * @see ObjectAdapter::waitForDeactivate
     *
     **/
    void waitForShutdown();

    /**
     *
     * Convert a string into a proxy. For example,
     * <literal>MyCategory/MyObject:tcp -h some_host -p
     * 10000</literal> creates a proxy that refers to the &Ice; object
     * having an identity with a name "MyObject" and a category
     * "MyCategory", with the server running on host "some_host", port
     * 10000.
     *
     * @param str The string to convert into a proxy.
     *
     * @return The proxy.
     *
     * @see proxyToString
     *
     **/
    Object* stringToProxy(string str);

    /**
     *
     * Convert a proxy into a string.
     *
     * @param obj The proxy to convert into a string.
     *
     * @return The "stringified" proxy.
     *
     * @see stringToProxy
     *
     **/
    string proxyToString(Object* obj);

    /**
     *
     * Create a new object adapter. The endpoints for the object
     * adapter are taken from the property
     * <literal><replaceable>name</replaceable>.Endpoints</literal>.
     *
     * @param name The object adapter name.
     *
     * @return The new object adapter.
     *
     * @see createObjectAdapterWithEndpoints
     * @see ObjectAdapter
     * @see Properties
     *
     **/
    ObjectAdapter createObjectAdapter(string name);

    /**
     *
     * Create a new object adapter with endpoints. This method sets
     * the property
     * <literal><replaceable>name</replaceable>.Endpoints</literal>,
     * and then calls [createObjectAdapter]. It is provided as a
     * convenience function.
     * 
     * @param name The object adapter name.
     *
     * @param endpoints The endpoints for the object adapter.
     *
     * @return The new object adapter.
     *
     * @see createObjectAdapter
     * @see ObjectAdapter
     * @see Properties
     *
     **/
    ObjectAdapter createObjectAdapterWithEndpoints(string name, string endpoints);

    /**
     *
     * Add a servant factory to this communicator. Installing a
     * factory with an id for which a factory is already registered
     * throws [AlreadyRegisteredException].
     *
     * @param factory The factory to add.
     *
     * @param id The type id for which the factory can create
     * instances.
     *
     * @see removeObjectFactory
     * @see findObjectFactory
     * @see ObjectFactory
     *
     **/
    void addObjectFactory(ObjectFactory factory, string id);

    /**
     *
     * Remove a servant factory from this communicator. Removing an id
     * for which no factory is registered throws [NotRegisteredException].
     *
     * @param id The type id for which the factory can create
     * instances.
     *
     * @see addObjectFactory
     * @see findObjectFactory
     * @see ObjectFactory
     *
     **/
    void removeObjectFactory(string id);

    /**
     *
     * Find a servant factory registered with this communicator.
     *
     * @param id The type id for which the factory can create
     * instances.
     *
     * @return The servant factory, or null if no servant factory was
     * found for the given id.
     *
     * @see addObjectFactory
     * @see removeObjectFactory
     * @see ObjectFactory
     *
     **/
    ObjectFactory findObjectFactory(string id);

    /**
     *
     * Get the properties for this communicator.
     *
     * @return This communicator's properties.
     *
     * @see Properties
     *
     **/
    Properties getProperties();

    /**
     *
     * Get the logger for this communicator.
     *
     * @return This communicator's logger.
     *
     * @see Logger
     *
     **/
    Logger getLogger();

    /**
     *
     * Set the logger for this communicator.
     *
     * @param log The logger to use for this communicator.
     *
     * @see Logger
     *
     **/
    void setLogger(Logger log);

    /**
     *
     * Get the statistics callback object for this communicator.
     *
     * @return This communicator's statistics callback object.
     *
     * @see Stats
     *
     **/
    Stats getStats();

    /**
     *
     * Set the statistics callback object for this communicator.
     *
     * @param st The statistics callback object to use for this
     * communicator.
     *
     * @see Stats
     *
     **/
    void setStats(Stats st);

    /**
     *
     * Set a default router for this communicator. All newly
     * created proxies will use this default router. To disable the
     * default router, null can be used. Note that this
     * operation has no effect on existing proxies.
     *
     * <note><para> You can also set a router for an individual proxy
     * by calling the operation [ice_router] on the proxy.</para></note>
     *
     * @param rtr The default router to use for this communicator.
     *
     * @see Router
     * @see ObjectAdapter::addRouter
     *
     **/
    void setDefaultRouter(Router* rtr);

    /**
     *
     * Set a default &Ice; locator for this communicator. All newly
     * created proxy and object adapters will use this default
     * locator. To disable the default locator, null can be used.
     * Note that this operation has no effect on existing proxies or
     * object adapters.
     *
     * <note><para> You can also set a locator for an individual proxy
     * by calling the operation [ice_locator] on the proxy, or for an
     * object adapter by calling the operation [setLocator] on the
     * object adapter.</para></note>
     *
     * @param loc The default locator to use for this communicator.
     *
     * @see Locator
     * @see ObjectAdapter::setLocator
     *
     **/
    void setDefaultLocator(Locator* loc);

    /**
     *
     * Get the plug-in manager for this communicator.
     *
     * @return This communicator's plug-in manager.
     *
     * @see PluginManager
     *
     **/
    PluginManager getPluginManager();
};

};

#endif
