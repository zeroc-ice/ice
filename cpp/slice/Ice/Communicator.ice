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
 * @see Logger
 * @see ObjectAdapter
 * @see Pickler
 * @see Properties
 * @see ValueFactory
 *
 **/
local class Communicator
{
    /**
     *
     * Destroy the Communicator. This operation calls
     * <literal>shutdown</literal> implicitly, if it hasn't been called
     * before. Calling <literal>destroy</literal> more than once is not
     * permissible, and will cause a local Ice exception to be
     * thrown. Calling <literal>destroy</literal> cleans up memory, and
     * shuts down the client-side of an application.
     *
     * @see shutdown
     *
     **/
    void destroy();

    /**
     *
     * Shut down the server-side of an
     * application. <literal>shutdown</literal> deactivates all Object
     * Adapters. Calling <literal>shutdown</literal> more than once is
     * not permissible, and will cause a local Ice exception to be
     * thrown.
     *
     * <note><para> Shutdown is not immediate, i.e., after
     * <literal>shutdown</literal> returns, the server-side of the
     * application might still be active. You can use
     * <literal>waitForShutdown</literal> to wait until shutdown is
     * complete. </para></note>
     *
     * @see destroy
     * @see waitForShutdown
     *
     **/
    void shutdown();

    /**
     *
     * Wait until the server-side of an application has shut
     * down. Calling <literal>shutdown</literal> initiates server-side
     * shutdown, and <literal>waitForShutdown</literal> only returns when
     * such shutdown has been completed. A typical use of this
     * operation is to call it from the main thread, which then waits
     * until some other thread calls <literal>shutdown</literal>. After
     * such shutdown is complete, the main thread returns and can do
     * some cleanup work before it calls <literal>destroy</literal> and
     * then exits the application.
     *
     * @see shutdown
     * @see destroy
     *
     **/
    void waitForShutdown();

    /**
     *
     * Convert a string into a Proxy. For example, <literal>MyObject:tcp
     * -h some_host -p 10000</literal> creates a proxy that refers to the
     * object with the identity "MyObject", with the server running at
     * host "some_host", port 10000.
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
     * Create a new object adapter. The endpoints for the object
     * adapter are taken from the property
     * <literal>ice.adapter.<replaceable>name</replaceable>.endpoints</literal>,
     * with <replaceable>name</replaceable> being the name of the
     * object adapter.
     *
     * @param name The name to use for the object adapter. This name
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
     * Create a new object adapter with a list of endpoints. In
     * contrast to <literal>createObjectAdapter</literal>, the endpoints to
     * use are passed explicitly as a parameter.
     *
     * @param name The name to use for the object adapter. This name
     * must be unique for the Communicator.
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
