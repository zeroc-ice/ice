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
#include <Ice/ValueFactoryF.ice>

module Ice
{

/**
 *
 * The central Ice object. One or more Communicators can be
 * instantiated for an Ice application. Communicator instantiation is
 * language specific, and not specified in Slice code.
 *
 **/
local class Communicator
{
    /**
     *
     * Destroy the Communicator. This operation calls
     * </code>shutdown()</code> implicitly, if it hasn't been called
     * before. Calling <code>destroy()</code> more than once is not
     * permissible, and will cause a local Ice exception to be
     * thrown. Calling <code>destroy()</code> cleans up memory, and
     * shuts down the client-side of an application.
     *
     * @see shutdown
     *
     **/
    void destroy();

    /**
     *
     * Shut down the server-side of an
     * application. <code>shutdown</code> deactivates all Object
     * Adapters. Calling <code>shutdown()</code> more than once is not
     * permissible, and will cause a local Ice exception to be
     * thrown. <em>Note:</em> shutdown is not immediate, i.e., after
     * <code>shutdown()</code> return, the server-side of the
     * application might still be active. You can use
     * <code>waitForShutdown</code> to wait until shutdown is
     * complete.
     *
     * @see destroy
     * @see waitForShutdown
     *
     **/
    void shutdown();

    /**
     *
     * Wait until the server-side of an application has shut
     * down. Calling <code>shutdown()</code> initiates server-side
     * shutdown, and <code>waitForShutdown()</code> only returns when
     * such shutdown has been completed. A typical use of this
     * operation is to call it from the main thread, which then waits
     * until some other thread calls <code>shutdown()</code>. After
     * such shutdown is complete, the main thread returns and can do
     * some cleanup work before it calls <code>destroy()</code> and
     * then exits the application.
     *
     * @see shutdown
     * @see destroy
     *
     **/
    void waitForShutdown();

    /**
     *
     * Convert a string into a Proxy. For example, <code>MyObject:tcp
     * -h some_host -p 10000</code> creates a proxy that refers to the
     * object with the identity "MyObject", with the server running at
     * host "some_host", port 10000.
     *
     * @param str The string to turn into a Proxy.
     *
     * @return The Proxy.
     *
     **/
    Object* stringToProxy(string str);

    /**
     *
     * Create a new object adapter. The endpoints for the object
     * adapter are taken from the property
     * <code>ice.adapter.<name>.endpoints</code>, with
     * <code><name></name> being the name of the object adapter.
     *
     * @param name The name to use for the object adapter.
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
     * Create a new object adapter with a list of endpoints. In
     * contrast to <code>createObjectAdapter</code>, the endpoints to
     * use are passed explicitly as a parameter.
     *
     * @param name The name to use for the object adapter.
     *
     * @param endpts The list of endpoints for the object adapter.
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
     * Install a new Value Factory.
     *
     * @param factory The factory to install.
     *
     * @param id The type id for which the factory can create instances.
     *
     **/
    void installValueFactory(ValueFactory factory, string id);

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
