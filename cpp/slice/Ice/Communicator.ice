// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
#include <Ice/ImplicitContextF.ice>
#include <Ice/Current.ice>

/**
 *
 * The Ice core library. Among many other features, the Ice core
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
 * The central object in Ice. One or more communicators can be
 * instantiated for an Ice application. Communicator instantiation
 * is language-specific, and not specified in Slice code.
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
     * this communicator's client functionality and destroys all object
     * adapters. Subsequent calls to [destroy] are ignored.
     *
     * @see shutdown
     * @see ObjectAdapter::destroy
     *
     **/
    void destroy();

    /**
     *
     * Shuts down this communicator's server functionality, which
     * includes the deactivation of all object adapters. (Attempts to use
     * a deactivated object adapter raise [ObjectAdapterDeactivatedException].)
     * Subsequent calls to [shutdown] are ignored.</p>
     *
     * <p class="Note"> After [shutdown] returns, no new requests are
     * processed. However, requests that have been started before
     * [shutdown] was called might still be active. You can use
     * [waitForShutdown] to wait for the completion of all
     * requests.
     *
     * @see destroy
     * @see waitForShutdown
     * @see ObjectAdapter::deactivate
     *
     **/
    void shutdown();

    /**
     *
     * Wait until the application has called [shutdown] (or [destroy]).
     * On the server side, this operation blocks the calling thread
     * until all currently-executing operations have completed.
     * On the client side, the operation simply block until another
     * thread has called [shutdown] or [destroy].
     *
     * <p>A typical use of this operation is to call it
     * from the main thread, which then waits until some other thread
     * calls [shutdown]. After shut-down is complete, the main thread
     * returns and can do some cleanup work before it finally calls
     * [destroy] to shut down the client functionality, and then
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
     * Check whether communicator has been shut down.
     *
     * @return True if the communicator has been shut down; false otherwise.
     *
     * @see shutdown
     *
     **/
    ["cpp:const"] bool isShutdown();

    /**
     *
     * Convert a string into a proxy. For example,
     * <tt>MyCategory/MyObject:tcp -h some_host -p
     * 10000</tt> creates a proxy that refers to the Ice object
     * having an identity with a name "MyObject" and a category
     * "MyCategory", with the server running on host "some_host", port
     * 10000. If the string does not parse correctly, the operation
     * throws [ProxyParseException].
     *
     * @param str The string to convert into a proxy.
     *
     * @return The proxy.
     *
     * @see proxyToString
     *
     **/
    ["cpp:const"] Object* stringToProxy(string str);

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
    ["cpp:const"] string proxyToString(Object* obj);


    /**
     * 
     * Convert a set of proxy properties into a proxy.
     *
     * @param property The base property name.
     * 
     * @return The proxy.
     *
     **/
    ["cpp:const"] Object* propertyToProxy(string property);

    /**
     *
     * Convert a string into an identity.
     *
     * @param str The string to convert into an identity.
     *
     * @return The identity.
     *
     * @see identityToString
     *
     **/
    ["cpp:const"] Identity stringToIdentity(string str);

    /**
     *
     * Convert an identity into a string.
     *
     * @param ident The identity to convert into a string.
     *
     * @return The "stringified" identity.
     *
     * @see stringToIdentity
     *
     **/
    ["cpp:const"] string identityToString(Identity ident);

    /**
     *
     * Create a new object adapter. The endpoints for the object
     * adapter are taken from the property <tt><em>name</em>.Endpoints</tt>.</p>
     *
     * <p>It is legal to create an object adapter with the empty string as
     * its name. Such an object adapter cannot be accessed remotely and, therefore,
     * is accessible only for collocated invocations that originate from the
     * the same communicator as is used by the adapter.</p>
     *
     * <p>Attempts to create a named object adapter for which no endpoint or
     * router configuration can be found raise [InitializationException].
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
     * Create a new object adapter with endpoints. This operation sets
     * the property <tt><em>name</em>.Endpoints</tt>,
     * and then calls [createObjectAdapter]. It is provided as a
     * convenience function.</p>
     *
     * <p>Calling this operation with an empty name or empty endpoint string
     * raises [InitializationException].
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
     * Create a new object adapter with a router. This operation
     * creates a routed object adapter.
     * 
     * @param name The object adapter name.
     *
     * @param router The router.
     *
     * @return The new object adapter.
     *
     * @see createObjectAdapter
     * @see ObjectAdapter
     * @see Properties
     *
     **/
    ObjectAdapter createObjectAdapterWithRouter(string name, Router* rtr);

    /**
     *
     * Add a servant factory to this communicator. Installing a
     * factory with an id for which a factory is already registered
     * throws [AlreadyRegisteredException].</p>
     *
     * <p>When unmarshaling an Ice object, the Ice run-time reads the
     * most-derived type id off the wire and attempts to create an
     * instance of the type using a factory. If no instance is created,
     * either because no factory was found, or because all factories
     * returned nil, the object is sliced to the next most-derived type
     * and the process repeats. If no factory is found that can create an
     * instance, the Ice run-time throws [NoObjectFactoryException].</p>
     *
     * <p>The following order is used to locate a factory for a type:</p>
     *
     * <ol>
     *
     * <li>The Ice run-time looks for a factory registered
     * specifically for the type.</li>
     *
     * <li>If no instance has been created, the Ice run-time looks
     * for the default factory, which is registered with an emtpy type id.
     * </li>
     *
     * <li>If no instance has been created by any of the preceding
     * steps, the Ice run-time looks for a factory that may have been
     * statically generated by the language mapping for non-abstract classes.
     * </li>
     *
     * </ol>
     * <p>
     *
     * @param factory The factory to add.
     *
     * @param id The type id for which the factory can create instances, or
     * an empty string for the default factory.
     *
     * @see findObjectFactory
     * @see ObjectFactory
     *
     **/
    void addObjectFactory(ObjectFactory factory, string id);

    /**
     *
     * Find a servant factory registered with this communicator.
     *
     * @param id The type id for which the factory can create instances,
     * or an empty string for the default factory.
     *
     * @return The servant factory, or null if no servant factory was
     * found for the given id.
     *
     * @see addObjectFactory
     * @see ObjectFactory
     *
     **/
    ["cpp:const"] ObjectFactory findObjectFactory(string id);


    /**
     *
     * Get the currently-set default context.
     *
     * <p class="Deprecated">This operation is deprecated as of version 3.2.
     *
     * @return The currently established default context. If no
     * default context is currently set, [getDefaultContext]
     * returns an empty context.
     *
     * @see setDefaultContext
     **/
    ["cpp:const", "deprecate:getDefaultContext is deprecated, use per-proxy contexts or implicit contexts (if applicable) instead."]
    Context getDefaultContext();

    /**
     *
     * Set a default context on this communicator. All newly
     * created proxies will use this default context. This operation 
     * has no effect on existing proxies.
     *
     * <p class="Note"> You can also set a context for an individual proxy
     * by calling the operation [ice_context] on the proxy.</p>
     *
     * <p class="Deprecated">This operation is deprecated as of version 3.2.
     *
     * @param ctx The default context to be set.
     * @see getDefaultContext
     **/
    ["deprecate:setDefaultContext is deprecated, use per-proxy contexts or implicit contexts (if applicable) instead."]
    void setDefaultContext(Context ctx);
    
    /**
     * Get the implicit context associated with this communicator.
     *
     * @return The implicit context associated with this communicator; 
     * returns null when the property Ice.ImplicitContext is not set 
     * or is set to None.
     *
     **/
    ["cpp:const"] ImplicitContext getImplicitContext();
    
    /**
     *
     * Get the properties for this communicator.
     *
     * @return This communicator's properties.
     *
     * @see Properties
     *
     **/
    ["cpp:const"] Properties getProperties();

    /**
     *
     * Get the logger for this communicator.
     *
     * @return This communicator's logger.
     *
     * @see Logger
     *
     **/
    ["cpp:const"] Logger getLogger();

    /**
     *
     * Get the statistics callback object for this communicator.
     *
     * @return This communicator's statistics callback object.
     *
     * @see Stats
     *
     **/
    ["cpp:const"] Stats getStats();

    /**
     *
     * Get the default router this communicator.
     *
     * @return The default router for this communicator.
     *
     * @see setDefaultRouter
     * @see Router
     *
     **/
    ["cpp:const"] Router* getDefaultRouter();

    /**
     *
     * Set a default router for this communicator. All newly
     * created proxies will use this default router. To disable the
     * default router, null can be used. Note that this
     * operation has no effect on existing proxies.</p>
     *
     * <p class="Note">You can also set a router for an individual proxy
     * by calling the operation [ice_router] on the proxy.
     *
     * @param rtr The default router to use for this communicator.
     *
     * @see getDefaultRouter
     * @see createObjectAdapterWithRouter
     * @see Router
     *
     **/
    void setDefaultRouter(Router* rtr);

    /**
     *
     * Get the default locator this communicator.
     *
     * @return The default locator for this communicator.
     *
     * @see setDefaultLocator
     * @see Locator
     *
     **/
    ["cpp:const"] Locator* getDefaultLocator();

    /**
     *
     * Set a default Ice locator for this communicator. All newly
     * created proxy and object adapters will use this default
     * locator. To disable the default locator, null can be used.
     * Note that this operation has no effect on existing proxies or
     * object adapters.</p>
     *
     * <p class="Note"> You can also set a locator for an individual proxy
     * by calling the operation [ice_locator] on the proxy, or for an
     * object adapter by calling the operation [setLocator] on the
     * object adapter.
     *
     * @param loc The default locator to use for this communicator.
     *
     * @see getDefaultLocator
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
    ["cpp:const"] PluginManager getPluginManager();

    /**
     *
     * Flush any pending batch requests for this communicator.
     * This causes all batch requests that were sent via proxies
     * obtained via this communicator to be sent to the server.
     *
     **/
    void flushBatchRequests();
};

};

#endif
