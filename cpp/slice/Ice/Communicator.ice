// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_COMMUNICATOR_ICE
#define ICE_COMMUNICATOR_ICE

#include <Ice/LoggerF.ice>
#include <Ice/ObjectAdapterF.ice>
#include <Ice/PropertiesF.ice>
#include <Ice/ObjectFactoryF.ice>
#include <Ice/UserExceptionFactoryF.ice>
#include <Ice/RouterF.ice>

/**
 *
 * The basic Ice module, containing all the Ice core functionality.
 *
 **/
module Ice
{
    
/**
 *
 * The central object in Ice. One or more Communicators can be
 * instantiated for an Ice application. Communicator instantiation is
 * language specific, and not specified in Slice code.
 *
 * @see Logger
 * @see ObjectAdapter
 * @see Pickler
 * @see Properties
 * @see ObjectFactory
 * @see UserExceptionFactory
 *
 **/
local interface Communicator
{
    /**
     *
     * Destroy the Communicator. This operation calls [shutdown]
     * implicitly.  Calling [destroy] cleans up memory, and shuts the
     * client-side of an application down. Subsequent calls to
     * [destroy] are ignored.
     *
     * @see shutdown
     *
     **/
    void destroy();

    /**
     *
     * Shut the server-side of an application down. [shutdown]
     * deactivates all Object Adapters. Subsequent calls to [shutdown]
     * are ignored.
     *
     * <important><para>Shutdown is the only operation that is
     * signal-safe, i.e., it is safe to call this operation from a
     * Unix signal handler. No other Ice operation can be called from
     * a Unix signal handler.</para></important>
     *
     * <note><para> Shutdown is not immediate, i.e., after [shutdown]
     * returns, the server-side of the application might still be
     * active. You can use [waitForShutdown] to wait until shutdown is
     * complete. </para></note>
     *
     * @see destroy
     * @see waitForShutdown
     * @see ObjectAdapter::deactivate
     *
     **/
    void shutdown();

    /**
     *
     * Wait until the server-side of an application has shut
     * down. Calling [shutdown] initiates server-side shutdown, and
     * [waitForShutdown] only returns when such shutdown has been
     * completed. A typical use of this operation is to call it from
     * the main thread, which then waits until some other thread calls
     * [shutdown]. After such shutdown is complete, the main thread
     * returns and can do some cleanup work before it finally calls
     * [destroy] to also shut the client-side of the application down,
     * and then exits the application.
     *
     * @see shutdown
     * @see destroy
     *
     **/
    void waitForShutdown();

    /**
     *
     * Convert a string into a Proxy. For example,
     * [MyCategory#MyObject:tcp -h some_host -p 10000] creates a proxy
     * that refers to the Ice Object having an identity with a name
     * "MyObject" and a category "MyCategory", with the server running
     * on host "some_host", port 10000.
     *
     * @param str The string to turn into a Proxy.
     *
     * @return The Proxy.
     *
     * @see proxyToString
     *
     **/
    Object* stringToProxy(string str);

    /**
     *
     * Convert a Proxy into a string.
     *
     * @param obj The Proxy to turn into a string.
     *
     * @return The "stringified" Proxy.
     *
     * @see stringToProxy
     *
     **/
    string proxyToString(Object* obj);

    /**
     *
     * Create a new Object Adapter. The endpoints for the Object
     * Adapter are taken from the property
     * [Ice.Adapter.<replaceable>name</replaceable>.Endpoints], with
     * <replaceable>name</replaceable> being the name of the Object
     * Adapter.
     *
     * @param name The name to use for the Object Adapter. This name
     * must be unique for the Communicator.
     *
     * @return The new Object Adapter.
     *
     * @see ObjectAdapter
     * @see Properties
     * @see createObjectAdapterFromProperty
     * @see createObjectAdapterWithEndpoints
     *
     **/
    ObjectAdapter createObjectAdapter(string name);

    /**
     *
     * Create a new Object Adapter from a property. The endpoints for
     * the Object Adapter are taken from the property
     * <replaceable>property</replaceable>.
     *
     * @param name The name to use for the Object Adapter. This name
     * must be unique for the Communicator.
     *
     * @param property The property from which the endpoints are taken.
     *
     * @return The new Object Adapter.
     *
     * @see Properties
     * @see ObjectAdapter
     * @see createObjectAdapterWithEndpoints
     *
     **/
    ObjectAdapter createObjectAdapterFromProperty(string name, string property);

    /**
     *
     * Create a new Object Adapter with a list of endpoints. In
     * contrast to [createObjectAdapter] and
     * [createObjectAdapterFromProperty], the endpoints to use are
     * passed explicitly as a parameter.
     *
     * @param name The name to use for the Object Adapter. This name
     * must be unique for the Communicator.
     *
     * @param endpts The list of endpoints for the Object Adapter.
     *
     * @return The new Object Adapter.
     *
     * @see ObjectAdapter
     * @see createObjectAdapter
     * @see createObjectAdapterFromProperty
     *
     **/
    ObjectAdapter createObjectAdapterWithEndpoints(string name, string endpts);

    /**
     *
     * Add a Servant factory to this Communicator. If a factory has
     * already been installed for the given id, the current factory
     * for this id is replaced by the new one.
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
     * Remove a Servant factory from this Communicator. This operation
     * does nothing if no factory for the given id has been installed.
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
     * Find a Servant factory installed with this Communicator.
     *
     * @param id The type id for which the factory can create
     * instances.
     *
     * @return The Servant factory, or null if no Servant factory was
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
     * Add a user exception factory to this Communicator. If a factory
     * has already been installed for the given id, the current
     * factory for this id is replaced by the new one.
     *
     * @param factory The factory to add.
     *
     * @param id The type id for which the factory can create user
     * exceptions.
     *
     * @see removeUserExceptionFactory
     * @see findUserExceptionFactory
     * @see UserExceptionFactory
     *
     **/
    void addUserExceptionFactory(UserExceptionFactory factory, string id);

    /**
     *
     * Remove a user exception factory from this Communicator. This
     * operation does nothing if no factory for the given id has been
     * installed.
     *
     * @param id The type id for which the factory can create user
     * exceptions.
     *
     * @see addUserExceptionFactory
     * @see findUserExceptionFactory
     * @see UserExceptionFactory
     *
     **/
    void removeUserExceptionFactory(string id);

    /**
     *
     * Find a user exception factory installed with this Communicator.
     *
     * @param id The type id for which the factory can create user
     * exceptions.
     *
     * @return The user exception factory, or null if no user
     * exception factory was found for the given id.
     *
     * @see addUserExceptionFactory
     * @see removeUserExceptionFactory
     * @see UserExceptionFactory
     *
     **/
    UserExceptionFactory findUserExceptionFactory(string id);

    /**
     *
     * Get the properties for this Communicator.
     *
     * @return This Communicator's properties.
     *
     * @see Properties
     *
     **/
    Properties getProperties();

    /**
     *
     * Get the logger for this Communicator.
     *
     * @return This Communicator's Logger.
     *
     * @see Logger
     *
     **/
    Logger getLogger();

    /**
     *
     * Set the logger for this Communicator.
     *
     * @param logger The logger to use for this Communicator.
     *
     * @see Logger
     *
     **/
    void setLogger(Logger logger);

    /**
     *
     * Set a default &Glacier; router for this Communicator. All newly
     * created Proxies will use this default router. To disable the
     * default router, null can be passed as argument. Note that this
     * operation has no effect on already existing Proxies.
     *
     * <note><para> You can also set a router for an individual Proxy
     * by calling the operation [ice_router] on such
     * Proxy.</para></note>
     *
     * @param router The default router to use for this Communicator.
     *
     * @see Router
     * @see ObjectAdapter::addRouter
     *
     **/
    void setDefaultRouter(Router* router);
};

};

#endif
