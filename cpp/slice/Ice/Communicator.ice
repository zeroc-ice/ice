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
#include <Ice/PicklerF.ice>
#include <Ice/PropertiesF.ice>
#include <Ice/ServantFactoryF.ice>

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
 * @see ServantFactory
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
     * Unix signal handler. No other Ice operation must be called from
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
     * Convert a string into a Proxy. For example, [MyObject:tcp -h
     * some_host -p 10000] creates a proxy that refers to the object
     * with the identity "MyObject", with the server running at host
     * "some_host", port 10000.
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
     * [ice.adapter.<replaceable>name</replaceable>.endpoints], with
     * <replaceable>name</replaceable> being the name of the Object
     * Adapter.
     *
     * @param name The name to use for the Object Adapter. This name
     * must be unique for the Communicator.
     *
     * @return The new Object Adapter.
     *
     * @see ObjectAdapter
     * @see createObjectAdapterWithEndpoints
     *
     **/
    ObjectAdapter createObjectAdapter(string name);

    /**
     *
     * Create a new Object Adapter with a list of endpoints. In
     * contrast to [createObjectAdapter], the endpoints to use are
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
     * @see removeServantFactory
     * @see findServantFactory
     * @see ServantFactory
     *
     **/
    void addServantFactory(ServantFactory factory, string id);

    /**
     *
     * Remove a Servant factory from this Communicator. This operation
     * does nothing if no factory for the given id has been installed.
     *
     * @param id The type id for which the factory can create
     * instances.
     *
     * @see addServantFactory
     * @see findServantFactory
     * @see ServantFactory
     *
     **/
    void removeServantFactory(string id);

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
     * @see addServantFactory
     * @see removeServantFactory
     * @see ServantFactory
     *
     **/
    ServantFactory findServantFactory(string id);

    /**
     *
     * Get the Properties for this Communicator.
     *
     * @return This Communicator's Properties.
     *
     **/
    Properties getProperties();

    /**
     *
     * Get the Logger for this Communicator.
     *
     * @return This Communicator's Logger.
     *
     **/
    Logger getLogger();

    /**
     *
     * Set the Logger for this Communicator.
     *
     * @param logger The logger to use for this Communicator.
     *
     **/
    void setLogger(Logger logger);

    /**
     *
     * Get the Pickler for this Communicator.
     *
     * @return This Communicator's Pickler.
     *
     **/
    Pickler getPickler();
};

};

#endif
