// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.Ice;

/**
 * A class that encapsulates data to initialize a communicator.
 *
 * @see Util#initialize
 * @see Properties
 * @see Logger
 **/
public final class InitializationData implements Cloneable
{
    /**
     * Creates an instance with all members set to <code>null</code>.
     **/
    public
    InitializationData()
    {
    }

    /**
     * Creates and returns a copy of this object.
     **/
    @Override
    public InitializationData
    clone()
    {
        //
        // A member-wise copy is safe because the members are immutable.
        //
        InitializationData c = null;
        try
        {
            c = (InitializationData)super.clone();
        }
        catch(CloneNotSupportedException ex)
        {
            assert false;
        }
        return c;
    }

    /**
     * The properties for the communicator.
     **/
    public Properties properties;

    /**
     * The logger for the communicator.
     **/
    public Logger logger;

    /**
     * The communicator observer used by the Ice run-time.
     **/
    public com.zeroc.Ice.Instrumentation.CommunicatorObserver observer;

    /**
     * threadStart is called whenever the communicator starts a new thread.
     **/
    public Runnable threadStart;

    /**
     * threadStop is called whenever a thread created by the communicator is
     * about to be destroyed.
     **/
    public Runnable threadStop;

    /**
     * The custom class loader for the communicator.
     **/
    public ClassLoader classLoader;

    /**
      * You can control which thread receives operation invocations and AMI
      * callbacks by supplying a dispatcher.
      * <p>
      * For example, you can use this dispatching facility to ensure that
      * all invocations and callbacks are dispatched in a GUI event loop
      * thread so that it is safe to invoke directly on GUI objects.
      * <p>
      * The dispatcher is responsible for running (dispatching) the
      * invocation or AMI callback on its favorite thread. It must execute the
      * the provided <code>Runnable</code> parameter. The con parameter represents
      * the connection associated with this dispatch.
      **/
    public java.util.function.BiConsumer<Runnable, Connection> dispatcher;

    /**
     * Applications that make use of compact type IDs to conserve space
     * when marshaling class instances, and also use the streaming API to
     * extract such classes, can intercept the translation between compact
     * type IDs and their corresponding string type IDs by installing a
     * compact ID resolver in <code>InitializationData</code>
     * The parameter id represents the compact ID; the returned value is the
     * type ID such as <code>"::Module::Class"</code>, or an empty string if
     * the compact ID is unknown.
     **/
    public java.util.function.IntFunction<String> compactIdResolver;

    /**
     * The batch request interceptor.
     **/
    public BatchRequestInterceptor batchRequestInterceptor;

    /**
     * The value factory manager.
     **/
    public ValueFactoryManager valueFactoryManager;
}
