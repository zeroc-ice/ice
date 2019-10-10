//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["ice-prefix"]]

[["js:module:ice"]]

[["objc:dll-export:ICE_API"]]
[["objc:header-dir:objc"]]

[["python:pkgdir:Ice"]]

#include <Ice/LoggerF.ice>
#include <Ice/InstrumentationF.ice>
#include <Ice/ObjectAdapterF.ice>
#include <Ice/ObjectFactory.ice>
#include <Ice/ValueFactory.ice>
#include <Ice/Router.ice>
#include <Ice/Locator.ice>
#include <Ice/PluginF.ice>
#include <Ice/ImplicitContextF.ice>
#include <Ice/Current.ice>
#include <Ice/Properties.ice>
#include <Ice/FacetMap.ice>
#include <Ice/Connection.ice>

/**
 *
 * The Ice core library. Among many other features, the Ice core
 * library manages all the communication tasks using an efficient
 * protocol (including protocol compression and support for both TCP
 * and UDP), provides a thread pool for multi-threaded servers, and
 * additional functionality that supports high scalability.
 *
 **/
#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

["objc:prefix:ICE"]
module Ice
{

/**
 *
 * The central object in Ice. One or more communicators can be
 * instantiated for an Ice application. Communicator instantiation
 * is language-specific, and not specified in Slice code.
 *
 * @see Logger
 * @see ObjectAdapter
 * @see Properties
 * @see ValueFactory
 *
 **/
["clr:implements:global::System.IDisposable", "java:implements:java.lang.AutoCloseable"]

#if !defined(__SLICE2PHP__) && !defined(__SLICE2MATLAB__)
local interface Communicator
{

#ifdef __SLICE2JAVA__
    /**
     *
     * Destroy the communicator. This Java-only method overrides close in
     * java.lang.AutoCloseable and does not throw any exception.
     *
     * @see #destroy
     *
     **/
    void close();
#endif

    /**
     *
     * Destroy the communicator. This operation calls {@link #shutdown}
     * implicitly.  Calling {@link #destroy} cleans up memory, and shuts down
     * this communicator's client functionality and destroys all object
     * adapters. Subsequent calls to {@link #destroy} are ignored.
     *
     * @see #shutdown
     * @see ObjectAdapter#destroy
     *
     **/
    ["cpp:noexcept", "swift:noexcept", "js:async"] void destroy();

    /**
     *
     * Shuts down this communicator's server functionality, which
     * includes the deactivation of all object adapters. Attempts to use a
     * deactivated object adapter raise ObjectAdapterDeactivatedException.
     * Subsequent calls to shutdown are ignored.
     *
     * After shutdown returns, no new requests are processed. However, requests
     * that have been started before shutdown was called might still be active.
     * You can use {@link #waitForShutdown} to wait for the completion of all
     * requests.
     *
     * @see #destroy
     * @see #waitForShutdown
     * @see ObjectAdapter#deactivate
     *
     **/
    ["cpp:noexcept", "swift:noexcept", "js:async"] void shutdown();

    /**
     *
     * Wait until the application has called {@link #shutdown} (or {@link #destroy}).
     * On the server side, this operation blocks the calling thread
     * until all currently-executing operations have completed.
     * On the client side, the operation simply blocks until another
     * thread has called {@link #shutdown} or {@link #destroy}.
     *
     * A typical use of this operation is to call it from the main thread,
     * which then waits until some other thread calls {@link #shutdown}.
     * After shut-down is complete, the main thread returns and can do some
     * cleanup work before it finally calls {@link #destroy} to shut down
     * the client functionality, and then exits the application.
     *
     * @see #shutdown
     * @see #destroy
     * @see ObjectAdapter#waitForDeactivate
     *
     **/
    ["cpp:noexcept", "swift:noexcept", "js:async"] void waitForShutdown();

    /**
     *
     * Check whether communicator has been shut down.
     *
     * @return True if the communicator has been shut down; false otherwise.
     *
     * @see #shutdown
     *
     **/
    ["cpp:const", "cpp:noexcept", "swift:noexcept"] bool isShutdown();

    /**
     *
     * Convert a stringified proxy into a proxy. For example,
     * <code>MyCategory/MyObject:tcp -h some_host -p
     * 10000</code> creates a proxy that refers to the Ice object
     * having an identity with a name "MyObject" and a category
     * "MyCategory", with the server running on host "some_host", port
     * 10000. If the stringified proxy does not parse correctly, the
     * operation throws one of ProxyParseException, EndpointParseException,
     * or IdentityParseException. Refer to the Ice manual for a detailed
     * description of the syntax supported by stringified proxies.
     *
     * @param str The stringified proxy to convert into a proxy.
     *
     * @return The proxy, or nil if <code>str</code> is an empty string.
     *
     * @see #proxyToString
     *
     **/
    ["cpp:const"] Object* stringToProxy(string str);

    /**
     *
     * Convert a proxy into a string.
     *
     * @param obj The proxy to convert into a stringified proxy.
     *
     * @return The stringified proxy, or an empty string if
     * <code>obj</code> is nil.
     *
     * @see #stringToProxy
     *
     **/
    ["cpp:const", "swift:noexcept"] string proxyToString(Object* obj);

    /**
     *
     * Convert a set of proxy properties into a proxy. The "base"
     * name supplied in the <code>property</code> argument refers to a
     * property containing a stringified proxy, such as
     * <code>MyProxy=id:tcp -h localhost -p 10000</code>. Additional
     * properties configure local settings for the proxy, such as
     * <code>MyProxy.PreferSecure=1</code>. The "Properties"
     * appendix in the Ice manual describes each of the supported
     * proxy properties.
     *
     * @param property The base property name.
     *
     * @return The proxy.
     *
     **/
    ["cpp:const"] Object* propertyToProxy(string property);

    /**
     *
     * Convert a proxy to a set of proxy properties.
     *
     * @param proxy The proxy.
     *
     * @param property The base property name.
     *
     * @return The property set.
     *
     **/
    ["cpp:const", "swift:noexcept"] PropertyDict proxyToProperty(["swift:nonnull"] Object* proxy, string property);

#ifndef __SLICE2SWIFT__
    /**
     *
     * Convert a string into an identity. If the string does not parse
     * correctly, the operation throws IdentityParseException.
     *
     * @param str The string to convert into an identity.
     *
     * @return The identity.
     *
     * @see #identityToString
     *
     **/
    ["cpp:const", "deprecate:stringToIdentity() is deprecated, use the static stringToIdentity() method instead."]
    Identity stringToIdentity(string str);
#endif

    /**
     *
     * Convert an identity into a string.
     *
     * @param ident The identity to convert into a string.
     *
     * @return The "stringified" identity.
     *
     * @see #stringToIdentity
     *
     **/
    ["cpp:const", "swift:noexcept"] string identityToString(Identity ident);

    /**
     *
     * Create a new object adapter. The endpoints for the object
     * adapter are taken from the property <code><em>name</em>.Endpoints</code>.
     *
     * It is legal to create an object adapter with the empty string as
     * its name. Such an object adapter is accessible via bidirectional
     * connections or by collocated invocations that originate from the
     * same communicator as is used by the adapter.
     *
     * Attempts to create a named object adapter for which no configuration
     * can be found raise InitializationException.
     *
     * @param name The object adapter name.
     *
     * @return The new object adapter.
     *
     * @see #createObjectAdapterWithEndpoints
     * @see ObjectAdapter
     * @see Properties
     *
     **/
    ["js:async", "swift:nonnull"] ObjectAdapter createObjectAdapter(string name);

    /**
     *
     * Create a new object adapter with endpoints. This operation sets
     * the property <code><em>name</em>.Endpoints</code>, and then calls
     * {@link #createObjectAdapter}. It is provided as a convenience
     * function.
     *
     * Calling this operation with an empty name will result in a
     * UUID being generated for the name.
     *
     * @param name The object adapter name.
     *
     * @param endpoints The endpoints for the object adapter.
     *
     * @return The new object adapter.
     *
     * @see #createObjectAdapter
     * @see ObjectAdapter
     * @see Properties
     *
     **/
    ["js:async", "swift:nonnull"] ObjectAdapter createObjectAdapterWithEndpoints(string name, string endpoints);

    /**
     *
     * Create a new object adapter with a router. This operation
     * creates a routed object adapter.
     *
     * Calling this operation with an empty name will result in a
     * UUID being generated for the name.
     *
     * @param name The object adapter name.
     *
     * @param rtr The router.
     *
     * @return The new object adapter.
     *
     * @see #createObjectAdapter
     * @see ObjectAdapter
     * @see Properties
     *
     **/
    ["js:async", "swift:nonnull"] ObjectAdapter
    createObjectAdapterWithRouter(string name, ["objc:param:router", "swift:nonnull"] Router* rtr);

#if !defined(__SLICE2SWIFT__)
    /**
     *
     * Add an object factory to this communicator. Installing a
     * factory with an id for which a factory is already registered
     * throws AlreadyRegisteredException.
     *
     * When unmarshaling an Ice object, the Ice run time reads the
     * most-derived type id off the wire and attempts to create an
     * instance of the type using a factory. If no instance is created,
     * either because no factory was found, or because all factories
     * returned nil, the behavior of the Ice run time depends on the
     * format with which the object was marshaled:
     *
     * If the object uses the "sliced" format, Ice ascends the class
     * hierarchy until it finds a type that is recognized by a factory,
     * or it reaches the least-derived type. If no factory is found that
     * can create an instance, the run time throws NoValueFactoryException.
     *
     * If the object uses the "compact" format, Ice immediately raises
     * NoValueFactoryException.
     *
     * The following order is used to locate a factory for a type:
     *
     * <ol>
     *
     * <li>The Ice run-time looks for a factory registered
     * specifically for the type.</li>
     *
     * <li>If no instance has been created, the Ice run-time looks
     * for the default factory, which is registered with an empty type id.
     * </li>
     *
     * <li>If no instance has been created by any of the preceding
     * steps, the Ice run-time looks for a factory that may have been
     * statically generated by the language mapping for non-abstract classes.
     * </li>
     *
     * </ol>
     *
     * @param factory The factory to add.
     *
     * @param id The type id for which the factory can create instances, or
     * an empty string for the default factory.
     *
     * @see #findObjectFactory
     * @see ObjectFactory
     * @see ValueFactoryManager#add
     *
     **/
    ["deprecate:addObjectFactory() is deprecated, use ValueFactoryManager::add() instead."]
    void addObjectFactory(ObjectFactory factory, ["objc:param:sliceId"] string id);

    /**
     *
     * Find an object factory registered with this communicator.
     *
     * @param id The type id for which the factory can create instances,
     * or an empty string for the default factory.
     *
     * @return The object factory, or null if no object factory was
     * found for the given id.
     *
     * @see #addObjectFactory
     * @see ObjectFactory
     * @see ValueFactoryManager#find
     *
     **/
    ["cpp:const", "cpp:noexcept", "deprecate:findObjectFactory() is deprecated, use ValueFactoryManager::find() instead."]
    ObjectFactory findObjectFactory(string id);

#endif

    /**
     * Get the implicit context associated with this communicator.
     *
     * @return The implicit context associated with this communicator;
     * returns null when the property Ice.ImplicitContext is not set
     * or is set to None.
     *
     **/
    ["cpp:const", "cpp:noexcept", "swift:noexcept", "swift:nonnull"]
    ImplicitContext getImplicitContext();

    /**
     *
     * Get the properties for this communicator.
     *
     * @return This communicator's properties.
     *
     * @see Properties
     *
     **/
    ["cpp:const", "cpp:noexcept", "swift:noexcept", "swift:nonnull"] Properties getProperties();

    /**
     *
     * Get the logger for this communicator.
     *
     * @return This communicator's logger.
     *
     * @see Logger
     *
     **/
    ["cpp:const", "cpp:noexcept", "swift:noexcept", "swift:nonnull"] Logger getLogger();

#if !defined(__SLICE2SWIFT__) && !defined(__SLICE2JS__)
    /**
     *
     * Get the observer resolver object for this communicator.
     *
     * @return This communicator's observer resolver object.
     *
     **/
    ["cpp:const", "cpp:noexcept"] Instrumentation::CommunicatorObserver getObserver();
#endif

    /**
     *
     * Get the default router this communicator.
     *
     * @return The default router for this communicator.
     *
     * @see #setDefaultRouter
     * @see Router
     *
     **/
    ["cpp:const", "swift:noexcept"] Router* getDefaultRouter();

    /**
     *
     * Set a default router for this communicator. All newly
     * created proxies will use this default router. To disable the
     * default router, null can be used. Note that this
     * operation has no effect on existing proxies.
     *
     * You can also set a router for an individual proxy
     * by calling the operation <code>ice_router</code> on the proxy.
     *
     * @param rtr The default router to use for this communicator.
     *
     * @see #getDefaultRouter
     * @see #createObjectAdapterWithRouter
     * @see Router
     *
     **/
    ["swift:noexcept"] void setDefaultRouter(Router* rtr);

    /**
     *
     * Get the default locator this communicator.
     *
     * @return The default locator for this communicator.
     *
     * @see #setDefaultLocator
     * @see Locator
     *
     **/
    ["cpp:const", "swift:noexcept"] Locator* getDefaultLocator();

    /**
     *
     * Set a default Ice locator for this communicator. All newly
     * created proxy and object adapters will use this default
     * locator. To disable the default locator, null can be used.
     * Note that this operation has no effect on existing proxies or
     * object adapters.
     *
     * You can also set a locator for an individual proxy by calling the
     * operation <code>ice_locator</code> on the proxy, or for an object adapter
     * by calling {@link ObjectAdapter#setLocator} on the object adapter.
     *
     * @param loc The default locator to use for this communicator.
     *
     * @see #getDefaultLocator
     * @see Locator
     * @see ObjectAdapter#setLocator
     *
     **/
    ["swift:noexcept"] void setDefaultLocator(Locator* loc);

#if !defined(__SLICE2SWIFT__) && !defined(__SLICE2JS__)
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
#endif

    /**
     *
     * Get the value factory manager for this communicator.
     *
     * @return This communicator's value factory manager.
     *
     * @see ValueFactoryManager
     *
     **/
    ["cpp:const", "cpp:noexcept", "swift:noexcept", "swift:nonnull"]
    ValueFactoryManager getValueFactoryManager();

    /**
     *
     * Flush any pending batch requests for this communicator.
     * This means all batch requests invoked on fixed proxies
     * for all connections associated with the communicator.
     * Any errors that occur while flushing a connection are ignored.
     *
     * @param compress Specifies whether or not the queued batch requests
     * should be compressed before being sent over the wire.
     *
     **/
    ["async-oneway"] void flushBatchRequests(CompressBatch compress);

#ifndef __SLICE2JS__
    /**
     *
     * Add the Admin object with all its facets to the provided object adapter.
     * If Ice.Admin.ServerId is set and the provided object adapter has a {@link Locator},
     * createAdmin registers the Admin's Process facet with the {@link Locator}'s {@link LocatorRegistry}.
     *
     * createAdmin call only be called once; subsequent calls raise InitializationException.
     *
     * @param adminAdapter The object adapter used to host the Admin object; if null and
     * Ice.Admin.Endpoints is set, create, activate and use the Ice.Admin object adapter.
     *
     * @param adminId The identity of the Admin object.
     *
     * @return A proxy to the main ("") facet of the Admin object. Never returns a null proxy.
     *
     * @see #getAdmin
     *
     **/
    ["swift:nonnull"] Object* createAdmin(ObjectAdapter adminAdapter, Identity adminId);

    /**
     *
     * Get a proxy to the main facet of the Admin object.
     *
     * getAdmin also creates the Admin object and creates and activates the Ice.Admin object
     * adapter to host this Admin object if Ice.Admin.Enpoints is set. The identity of the Admin
     * object created by getAdmin is {value of Ice.Admin.InstanceName}/admin, or {UUID}/admin
     * when Ice.Admin.InstanceName is not set.
     *
     * If Ice.Admin.DelayCreation is 0 or not set, getAdmin is called by the communicator
     * initialization, after initialization of all plugins.
     *
     * @return A proxy to the main ("") facet of the Admin object, or a null proxy if no
     * Admin object is configured.
     *
     * @see #createAdmin
     **/
    ["cpp:const"] Object* getAdmin();

    /**
     *
     * Add a new facet to the Admin object.
     * Adding a servant with a facet that is already registered
     * throws AlreadyRegisteredException.
     *
     * @param servant The servant that implements the new Admin facet.
     * @param facet The name of the new Admin facet.
     *
     **/
    void addAdminFacet(["swift:nonnull"] Object servant, string facet);

    /**
     *
     * Remove the following facet to the Admin object.
     * Removing a facet that was not previously registered throws
     * NotRegisteredException.
     *
     * @param facet The name of the Admin facet.
     * @return The servant associated with this Admin facet.
     *
     **/
    ["swift:nonnull", "swift:attribute:@discardableResult"] Object removeAdminFacet(string facet);

    /**
     *
     * Returns a facet of the Admin object.
     *
     * @param facet The name of the Admin facet.
     * @return The servant associated with this Admin facet, or
     * null if no facet is registered with the given name.
     *
     **/
    ["swift:noexcept"] Object findAdminFacet(string facet);

    /**
     *
     * Returns a map of all facets of the Admin object.
     *
     * @return A collection containing all the facet names and
     * servants of the Admin object.
     *
     * @see #findAdminFacet
     *
     **/
    ["swift:noexcept"] FacetMap findAllAdminFacets();
#endif

#if defined(__SLICE2SWIFT__) || defined(ICE_SWIFT)
    /**
     *
     * Returns the client dispatch queue.
     *
     * @return The dispatch queue associated wih this Communicator's
     * client thread pool.
     *
     **/
    ["cpp:const", "swift:nonnull", "cpp:type:dispatch_queue_t", "swift:type:Dispatch.DispatchQueue"]
    LocalObject getClientDispatchQueue();

    /**
     *
     * Returns the server dispatch queue.
     *
     * @return The dispatch queue associated wih the Communicator's
     * server thread pool.
     *
     **/
    ["cpp:const", "swift:nonnull", "cpp:type:dispatch_queue_t", "swift:type:Dispatch.DispatchQueue"]
    LocalObject getServerDispatchQueue();
#endif
}

#endif

/**
 * The output mode for xxxToString method such as identityToString and proxyToString.
 * The actual encoding format for the string is the same for all modes: you
 * don't need to specify an encoding format or mode when reading such a string.
 *
 **/
local enum ToStringMode
{
    /**
     * Characters with ordinal values greater than 127 are kept as-is in the resulting string.
     * Non-printable ASCII characters with ordinal values 127 and below are encoded as \\t, \\n (etc.)
     * or \\unnnn.
     **/
    Unicode,

    /**
     * Characters with ordinal values greater than 127 are encoded as universal character names in
     * the resulting string: \\unnnn for BMP characters and \\Unnnnnnnn for non-BMP characters.
     * Non-printable ASCII characters with ordinal values 127 and below are encoded as \\t, \\n (etc.)
     * or \\unnnn.
     **/
    ASCII,

    /**
     * Characters with ordinal values greater than 127 are encoded as a sequence of UTF-8 bytes using
     * octal escapes. Characters with ordinal values 127 and below are encoded as \\t, \\n (etc.) or
     * an octal escape. Use this mode to generate strings compatible with Ice 3.6 and earlier.
     **/
    Compat
}

}
